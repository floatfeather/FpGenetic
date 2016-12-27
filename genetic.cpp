#include "status.h"
#include "genetic.h"
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <vector>
#include <iostream>

using namespace std;

#define ulong unsigned long int
#define Frac64 0x000FFFFFFFFFFFFF
#define Exp64  0x7FF0000000000000
#define Sign64 0x8000000000000000

namespace genetic {
namespace {
int pid_;
void handler(int n) {
	kill(pid_, SIGKILL);
	printf("kill: %d", pid_);
}

int GeneCmp(const Gene* a, const Gene* b) {
	if ((a->exp_ > b->exp_) || ((a->exp_ == b->exp_) && (a->frac_ > b->frac_)))
		return 1;
	return 0;
}
} // namespace

// RunnerOptions
RunnerOptions::RunnerOptions() {
	max_init_populations_ = 120;
	max_populations_ = 200;
	execution_timeout_ = 5;
}

string RunnerOptions::Filename() const{
	return filename_;
}

int RunnerOptions::MaxInitPopulations() const{
	return max_init_populations_;
}

int RunnerOptions::MaxPopulations() const{
	return max_populations_;
}

int RunnerOptions::ExecutionTimeout() const{
	return execution_timeout_;
}

void RunnerOptions::SetFilename(const string& f) {
	this->filename_ = f;
}

void RunnerOptions::SetMaxInitPopulations(int m) {
	this->max_init_populations_ = m;
}

void RunnerOptions::SetMaxPopulations(int m) {
	this->max_populations_ = m;
}

void RunnerOptions::SetExecutionTimeout(int m) {
	this->execution_timeout_ = m;
}

RunnerOptions& RunnerOptions::operator=(const RunnerOptions& rhs){
	this->max_init_populations_ = rhs.MaxInitPopulations();
	this->max_populations_ = rhs.MaxPopulations();
	this->execution_timeout_ = rhs.ExecutionTimeout();
	this->filename_ = rhs.Filename();
	return *this;
}

// Runner
Runner::Runner(const RunnerOptions& op) {
	options_ = op;
	generated_tests_ = 0;
	best_gene_ = NULL;
};

Runner::~Runner() {
	for(auto gene : genes_) {
		for(auto arg : gene->arguments_) {
			delete arg;
		}
		delete gene;
	}
}

Status Runner::ReadFunctionMetadata() {
	cout << "Reading types of arguments..." << endl;
	string meta_filename = "others/" + options_.Filename() + ".c.meta";
	ifstream meta_in(meta_filename.c_str());
	string type;
	while(meta_in >> type) {
		cout << type << " ";
		if (type == "double") {
			double_cnt_++;
			arg_types_.push_back(double_type);
		} else if (type == "int") {
			int_cnt_++;
			arg_types_.push_back(int_type);
		} else if (type == "gsl_mode_t") {
			// TODO
		} else {
			cout << endl;
			return Status("Invalid type" + type);
		}
	}
	cout << endl;
	return Status();
}

ulong Runner::RandUlong() {
	if (RAND_MAX != 0x7FFFFFFF)
		printf("Check your RAND_MAX.\n");
	ulong sign, t1, t;
	sign = rand() % 2;
	t1 = rand();
	t1 = t1 | (sign<<31);
	sign = rand() % 2;
	t = 0;
	t = rand();
	t = t | (sign<<31);
	t = (t << 32);
	t = (t | t1);
	return t;
}

void Runner::TestGene(Gene* gene) {
	int k;
	if(pipe(pipefd_) < 0) {
		printf("Failed to create pipe!\n");
		exit(0);
	}
	pid_ = fork();
	if (pid_ == 0) {
		RunFPDebug(*gene);
	} else {
		printf("input: ");
		for (auto arg : gene->arguments_) {
			printf("%s ", arg->DebugString().c_str());
		}
		printf("\n");
		signal(SIGALRM, handler);
		alarm(options_.ExecutionTimeout());
		waitpid(pid_, NULL, 0);
		GetRelativeError(gene);
		printf("relative error:  %lfe%d\n", gene->frac_, gene->exp_);
		UpdateMaxError(gene);
	}
}

void Runner::RunFPDebug(const Gene& gene) {
	close(pipefd_[0]);
	if (dup2(pipefd_[1], 1) == -1) {
		printf("dup2 error\n");
	}
	if (dup2(pipefd_[1], 2) == -1) {
		printf("dup2 error2\n");
	}
	char execfile[100] = "./others/";
	strcat(execfile, options_.Filename().c_str());
	char arg_str[5][100];
	for(int i = 0; i < gene.arguments_.size(); i++) {
		gene.arguments_[i]->ToString(arg_str[i]);
	}
	char script[100] = "/home/lillian/work/install_fpdebug/valgrind-3.7.0/fpdebug/script/fpdebug.sh";
	switch(gene.arguments_.size()) {
		case 1:
			if (execlp("bash", "bash", script, execfile, arg_str[0], NULL) < 0) {
				printf("error\n");
				exit(1);
			}
			break;
		
		case 2:
			if (execlp("bash", "bash", script, execfile, arg_str[0], arg_str[1], NULL) < 0) {
				printf("error\n");
				exit(1);
			}
			break;
		case 3:
			if (execlp("bash", "bash", script, execfile, arg_str[0], arg_str[1], arg_str[2], NULL) < 0) {
				printf("error\n");
				exit(1);
			}
			break;
		case 4:
			if (execlp("bash", "bash", script, execfile,
				arg_str[0], arg_str[1], arg_str[2], arg_str[3], NULL) < 0) {
				printf("error\n");
				exit(1);
			}
			break;
		case 5:
			if (execlp("bash", "bash", script, execfile,
				arg_str[0], arg_str[1], arg_str[2], arg_str[3], arg_str[4], NULL) < 0) {
				printf("error\n");
				exit(1);
			}
			break;
	}
	close(pipefd_[1]);
}

void Runner::UpdateMaxError(Gene* gene) {
	if (best_gene_ == NULL || gene->exp_ > best_gene_->exp_ 
		|| (gene->exp_ == best_gene_->exp_ && gene->frac_ > best_gene_->frac_)) {
		best_gene_ = gene;
	}
}

void Runner::GetRelativeError(Gene* gene) {
	char buf[8192];
	close(pipefd_[1]);
	memset(buf, 0, sizeof(buf));
	// read(pipefd_[0], buf, 8192);
	close(pipefd_[0]);
	// char *pos1;
	// pos1 = strstr(buf, "RELATIVE ERROR:");
	// if (pos1  == NULL)
	// {
		// gene->frac_ = 0;
		// gene->exp_ = -65536;
		// return;
	// }
	// pos1 += strlen("RELATIVE ERROR:");
	FILE * f = fopen("fpdebug_relerr.log", "r");
	fread(buf, 1, 8192, f);
	sscanf(buf, "%lf", &(gene->frac_));
	if (strchr(buf, '@') != NULL || gene->frac_ < 1)
	{
		gene->frac_ = 0;
		gene->exp_ = -65536;

		return;
	}
	char * pos2 = strchr(buf,'^');
	sscanf(pos2+1, "%d", &(gene->exp_));
}

ulong Runner::CreateDouble(int sign, int expt) {
	ulong ufrac = RandUlong();
	ulong uexp  =  ((ulong)expt) << 52;
	ulong usign = ((ulong)sign)<< 63;
	return ((usign & Sign64) | (uexp & Exp64) | (ufrac & Frac64));
}

void Runner::InitExtremeValues(int i, vector<int>* values) {
	if (i == double_cnt_ + int_cnt_) {
		Gene* g = new Gene();
		for(int j = 0; j < arg_types_.size(); j++) {
			if (arg_types_[j] == int_type) {
				int v = (1 << (*values)[j]) + rand() % 50;
				if (rand() % 2 == 1) v = -v;
				g->arguments_.push_back(new Argument(int_type, &v));
			} else if (arg_types_[j] == double_type) {
				int sign;
				sign = rand() % 2;
				if ((*values)[j] != 0 && (*values)[j] != 2046) {
					(*values)[j] += rand() % 50 - 25;
					if ((*values)[j] > 2046) (*values)[j] = 2046;
					if ((*values)[j] < 0) (*values)[j] = 0;
				}
				ulong temp = CreateDouble(sign, (*values)[j]);
				g->arguments_.push_back(new Argument(double_type, &temp));
			}
		}
		genes_.push_back(g);
		generated_tests_++;
		return;
	}
	if (arg_types_[i] == int_type) {
		values->push_back(0);
		InitExtremeValues(i + 1, values);
		(*values)[i] = 31;
		InitExtremeValues(i + 1, values);
	} else if (arg_types_[i] == double_type) {
		values->push_back(0);
		InitExtremeValues(i + 1, values);
		(*values)[i] = 2046;
		InitExtremeValues(i + 1, values);
		(*values)[i] = 1024;
		InitExtremeValues(i + 1, values);
	}
}

void Runner::InitPopulations() {
	cout << "Initialize populations..." << endl;
	vector<int> values;
	cout << "Initialize extreme values..." << endl;
	InitExtremeValues(0, &values);
	for(int i = 0; i < options_.MaxInitPopulations() - generated_tests_; i++) {
		Gene* g = new Gene();
		for(auto t : arg_types_) {
			if (t == double_type) {
				ulong temp = GetDouble();
				Argument* arg = new Argument(double_type, &temp);
				g->arguments_.push_back(arg);
			} else if (t == int_type) {
				int temp = GetInt();
				Argument* arg = new Argument(int_type, &temp);
				g->arguments_.push_back(arg);
			}
		}
		genes_.push_back(g);
	}
	cout << "Populations initialized." << endl;
}

Gene Runner::GetBestGene() {
	return *best_gene_;
}

void Runner::PrintGenes() {
	for(int i = 0; i < genes_.size(); i++) {
		printf("%d: ", i);
		for(auto arg : genes_[i]->arguments_) {
			char buf[100];
			arg->ToString(buf);
			printf("%s ", buf);
		}
		printf("\n");
	}
}

GeneticRunner::GeneticRunner(const RunnerOptions& op) : Runner(op) {}

Status GeneticRunner::Start() {
	cout << "Start!" << endl;
	Status read_status = ReadFunctionMetadata();
	if (!read_status.OK()) return read_status;
	InitDoublePool();
	InitIntPool();
	InitPopulations();
	//PrintGenes();
	for(auto gene: genes_) {
		TestGene(gene);
	}
	Evolution();
}

void GeneticRunner::Evolution() {
	while(generated_tests_ < options_.MaxPopulations()) {
		GroupGenes();
		Gene* g = RandomPick();
		cout << g->frac_ << " " << g->exp_ << endl;
		Gene* new_gene = Mutate(*g);
		TestGene(new_gene);
		generated_tests_++;
	}
}

Gene* GeneticRunner::Mutate(const Gene& gene) {
	Gene * new_gene = new Gene();
	cout << "Mutating..." << endl;
	for(auto arg : gene.arguments_) {
		cout << arg->DebugString() << " -> ";
		if (arg->type_ == double_type) {
			ulong value = *(ulong*)arg->Val();
			int expt = (value & Exp64) >> 52;
			if (expt >= 980 && expt <= 1070) {
				expt = expt + rand() % 10 - 5;
			} else {
				expt = expt + rand() % 100 - 50;
				if (expt < 0) {
					expt = 0;
				}
				if (expt > 2046) {
					expt = 2046;
				}
			}
			int sign = (value & Sign64) >> 63;
			if (rand() % 10 == 0) sign = 1 - sign;
			ulong temp = CreateDouble(sign, expt);
			new_gene->arguments_.push_back(new Argument(double_type, &temp));
		} else if (arg->type_ == int_type) {
			int new_value = 0; // TODO: overflow and underflow
			int value = *(int*)arg->Val();
			/*if (abs(value) < 1024) {
				new_value = value + rand() % 100 - 50;
			} else if (abs(value) < 1024 * 1024) {
				new_value = value + rand() % 40000 - 20000;
			} else {
				new_value = value + rand() % 40000000 - 20000000;
			}*/
			new_value = value + rand() % value - (value / 2);
			if (rand() % 10 == 0) new_value = -new_value;
			new_gene->arguments_.push_back(new Argument(int_type, &new_value));
		}
		cout << new_gene->arguments_[new_gene->arguments_.size() - 1]->DebugString() << endl;
	}
	genes_.push_back(new_gene);
	return new_gene;
}

void GeneticRunner::GroupGenes() {
	sort(genes_.begin(), genes_.end(), GeneCmp);
	if (genes_.empty()) return;
	stages_.clear();
	Stage st;
	st.max_gene_ = genes_[0];
	st.elements_.push_back(genes_[0]);
	double rest_weight = 1;
	st.weight_ = rest_weight * 0.6;
	stages_.push_back(st);
	rest_weight -= st.weight_;
	int j = 0;
	cout << "Stage " << j << ":" << st.max_gene_->exp_ << endl;
	for(int i = 1; i < genes_.size() && i < options_.MaxInitPopulations(); i++) {
		if (genes_[i]->exp_ < stages_[j].max_gene_->exp_ - 1) {
			Stage stg;
			stg.max_gene_ = genes_[i];
			stg.elements_.push_back(genes_[i]);
			stg.weight_ = rest_weight * 0.6;
			rest_weight -= stg.weight_;
			stages_.push_back(stg);
			j++;
			cout << "Stage " << j << ":" << stg.max_gene_->exp_ << endl;
		} else {
			stages_[j].elements_.push_back(genes_[i]);
		}
	}
	stages_[j].weight_ += rest_weight;
}

Gene* GeneticRunner::RandomPick() {
	double p = (double)rand() / RAND_MAX;
	double cntp = 0;
	for (int i = 0; i < stages_.size(); i++)
	{
		cntp += stages_[i].weight_;
		if (cntp > p)
		{
			return stages_[i].elements_[rand() % stages_[i].elements_.size()];
		}
	}

	/* never reach */
	printf("Error in randompick.\n");
	return stages_[0].elements_[0];
}

void GeneticRunner::InitDoublePool() {
	cout << "Initialize double pool..." << endl;
	exp_pool_.clear();
	exp_pool_.push_back(0);
	exp_pool_.push_back(2046);

	for (int i = 50; i < 980; i += 100) {
		exp_pool_.push_back(i + rand()%100 - 50);
	}

	for (int i = 980; i < 1070; i += 5) {
		exp_pool_.push_back(i + rand() % 10 - 5);
	}

	for (int i = 1070; i < 1997; i += 100) {
		exp_pool_.push_back(i + rand() % 100 - 50);
	}
}

void GeneticRunner::InitIntPool() {
	cout << "Initialize int pool..." << endl;
	int_pool_.clear();
	/*int gap = 1024 / 10;
	for(int i = 0; i < 1024; i += gap) {
		int_pool_.push_back(i + rand() % gap - (gap / 2));
	}
	gap = (((1 << 20)) - 1024) / 20;
	for(int i = gap; i <= (1 << 20); i += gap) {
		int_pool_.push_back(i + rand() % gap - (gap / 2));
	}
	gap = ((1 << 30) - (1 << 20)) / 30;
	for(int i = gap; i <= (1 << 30); i += gap) {
		int_pool_.push_back(i + rand() % gap - (gap / 2));
	}*/
	for(int i = 0; i < 32; i++) {
		int_pool_.push_back((1 << i));
		int_pool_.push_back(-(1 << i));
	}
}

ulong GeneticRunner::GetDouble() {
	int sign, expt;
	sign = rand() % 2;
	expt = exp_pool_[rand() % exp_pool_.size()];
	if (expt >= 980 && expt < 1070) {
		expt += (rand() % 20 - 10);
	} else {
		expt += (rand() % 200 - 100);
	}
	if (expt > 2046) expt = 2046;
	if (expt < 0) expt = 0;
	return CreateDouble(sign, expt);
}

int GeneticRunner::GetInt() {
	int value = int_pool_[rand() % int_pool_.size()];
	if (rand() % 2 == 1) return -value;
	else return value;
}

RandomRunner::RandomRunner(const RunnerOptions& op) : Runner(op) {}

Status RandomRunner::Start() {
	cout << "Start!" << endl;
	Status read_status = ReadFunctionMetadata();
	if (!read_status.OK()) return read_status;
	InitPopulations();
	//PrintGenes();
	for(auto gene: genes_) {
		TestGene(gene);
	}
}

ulong RandomRunner::GetDouble() {
	return RandUlong();
}

int RandomRunner::GetInt() {
	int value = rand();
	if (rand() % 2 == 1) return -value;
	else return value;
}

InputRunner::InputRunner(const RunnerOptions& op) : Runner(op) {}

Status InputRunner::Start() {
	cout << "Start!" << endl;
	Status read_status = ReadFunctionMetadata();
	if (!read_status.OK()) return read_status;
	InitDoublePool();
	InitIntPool();
	InitPopulations();
	DumpPopulations();
}

void InputRunner::InitDoublePool() {
	cout << "Initialize double pool..." << endl;
	exp_pool_.clear();
	exp_pool_.push_back(0);
	exp_pool_.push_back(2046);

	for (int i = 50; i < 980; i += 100) {
		exp_pool_.push_back(i + rand()%100 - 50);
	}

	for (int i = 980; i < 1070; i += 5) {
		exp_pool_.push_back(i + rand() % 10 - 5);
	}

	for (int i = 1070; i < 1997; i += 100) {
		exp_pool_.push_back(i + rand() % 100 - 50);
	}
}

void InputRunner::InitIntPool() {
	cout << "Initialize int pool..." << endl;
	int_pool_.clear();
	for(int i = 0; i < 32; i++) {
		int_pool_.push_back((1 << i));
		int_pool_.push_back(-(1 << i));
	}
}


ulong InputRunner::GetDouble() {
	int sign, expt;
	sign = rand() % 2;
	expt = exp_pool_[rand() % exp_pool_.size()];
	if (expt >= 980 && expt < 1070) {
		expt += (rand() % 20 - 10);
	} else {
		expt += (rand() % 200 - 100);
	}
	if (expt > 2046) expt = 2046;
	if (expt < 0) expt = 0;
	return CreateDouble(sign, expt);
}

int InputRunner::GetInt() {
	int value = int_pool_[rand() % int_pool_.size()];
	int new_value = value + rand() % value - (value / 2);
	return new_value;
}

void InputRunner::DumpPopulations() {
	string outputFile = "input/" + options_.Filename() + "_input.txt";
	FILE * output = fopen(outputFile.c_str(), "w");
	for(auto gene : genes_) {
		for(auto arg : gene->arguments_) {
			if (arg->type_ == double_type) {
				double dv = *(double*)arg->Val();
				fprintf(output, "%.100le ", dv);
			} else if (arg->type_ == int_type) {
				int iv = *(int*)arg->Val();
				fprintf(output, "%d ", iv);
			}
		}
		fprintf(output, "\n");
	}
	fclose(output);
}

}