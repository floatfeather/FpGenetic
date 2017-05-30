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
#define uint unsigned int
#define Frac64 0x000FFFFFFFFFFFFF
#define Exp64  0x7FF0000000000000
#define Sign64 0x8000000000000000
#define Frac32 0x007FFFFF
#define Exp32  0x7F800000
#define Sign32 0x80000000

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
	// max_populations_ = 1000;
	max_populations_ = 200;
	execution_timeout_ = 5;
	max_extreme_values_ = 5;
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

int RunnerOptions::MaxExtremeValues() const {
	return max_extreme_values_;
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

void RunnerOptions::SetMaxExtremeValues(int m) {
	this->max_extreme_values_ = m;
}

RunnerOptions& RunnerOptions::operator=(const RunnerOptions& rhs){
	this->max_init_populations_ = rhs.MaxInitPopulations();
	this->max_populations_ = rhs.MaxPopulations();
	this->execution_timeout_ = rhs.ExecutionTimeout();
	this->filename_ = rhs.Filename();
	this->max_extreme_values_ = rhs.MaxExtremeValues();
	return *this;
}

// Runner
Runner::Runner(const RunnerOptions& op, const string project) {
	options_ = op;
	generated_tests_ = 0;
	best_gene_ = NULL;
	project_ = project;
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
	string meta_filename = "others/" + project_ + "/" + options_.Filename() + ".c.meta";
	ifstream meta_in(meta_filename.c_str());
	string type;
	while(meta_in >> type) {
		cout << type << " ";
		Type t;
		if (type == "unsigned") {
			t.is_unsigned_ = true;
			meta_in >> type;
		}
		if (type == "array") {
			t.is_array_ = true;
			meta_in >> type;
		}
		if (type == "double") {
			t.base_ = double_type;
		} else if (type == "int") {
			t.base_ = int_type;
		} else if (type == "float") {
			t.base_ = float_type;
		} else if (type == "short") {
			t.base_ = short_type;
		} else if (type == "long") {
			t.base_ = long_type;
		} else if (type == "gsl_mode_t") {
			// TODO
		} else {
			cout << endl;
			return Status("Invalid type" + type);
		}
		arg_types_.push_back(t);
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

uint Runner::RandUint() {
	if (RAND_MAX != 0x7FFFFFFF)
		printf("Check your RAND_MAX.\n");
	uint sign, t1;
	sign = rand() % 2;
	t1 = rand();
	t1 = t1 | (sign<<31);
	return t1;
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
			char output[500];
			arg->ToString(output);
			printf("%s ", output);
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
	strcat(execfile, project_.c_str());
	strcat(execfile, "/");
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
	if (f == NULL) {
		cout << "Fail to open file fpdebug_relerr.log!" << endl;
		gene->frac_ = 0;
		gene->exp_ = -65536;
		return;
	}
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
	fclose(f);
}

ulong Runner::CreateDouble(int sign, int expt) {
	return CreateDouble(sign, expt, RandUlong());
}

ulong Runner::CreateDouble(int sign, int expt, ulong ufrac) {
	ulong uexp  =  ((ulong)expt) << 52;
	ulong usign = ((ulong)sign)<< 63;
	return ((usign & Sign64) | (uexp & Exp64) | (ufrac & Frac64));
}

uint Runner::CreateFloat(int sign, int expt) {
	return CreateFloat(sign, expt, RandUint());
}

uint Runner::CreateFloat(int sign, int expt, uint ufrac) {
	uint uexp  =  ((uint)expt) << 23;
	uint usign = ((uint)sign)<< 31;
	return ((usign & Sign32) | (uexp & Exp32) | (ufrac & Frac32));
}

ulong Runner::GetDoubleExtVal() {
	int r = rand() % 2;
	int expt = 0;
	int sign = rand() % 2;
	if (r == 0) {
		expt = 0;
	} else {
		expt = 2046;
	}
	return CreateDouble(sign, expt);
}

uint Runner::GetFloatExtVal() {
	int r = rand() % 2;
	int expt = 0;
	int sign = rand() % 2;
	if (r == 0) {
		expt = 0;
	} else {
		expt = 254;
	}
	return CreateFloat(sign, expt);
}

int Runner::GetIntExtVal(bool is_unsigned = false) {
	int value = 0;
	if (!is_unsigned) {
		if (rand() % 3 != 0) {	
			value = (1 << 30) + rand() % (1 << 29) - (1 << 28);
			if (rand() % 2 == 1) value = -value;
		}
	} else {
		if (rand() % 2) {	
			value = (1 << 31) + rand() % (1 << 30) - (1 << 29);
		}
	}
	return value;
}

short Runner::GetShortExtVal(bool is_unsigned = false) {
	int value = 0;
	if (!is_unsigned) {
		if (rand() % 3 != 0) {
			value = (1 << 14) + rand() % (1 << 13) - (1 << 12);
			if (rand() % 2 == 1) value = -value;
		}
	} else {
		value = (1 << 15) + rand() % (1 << 14) - (1 << 13);
	}
	return value;
}

long Runner::GetLongExtVal(bool is_unsigned = false) {
	int value = 0;
	if (!is_unsigned) {
		if (rand() % 2) {
			value = (1L << 62) + (long) RandUlong() % (1L << 61) - (1L << 60);
			if (rand() % 2 == 1) value = -value;
		}
	} else {
		value = (1L << 63) + (long) RandUlong() % (1L << 62) - (1L << 61);
	}
	return value;
}

void Runner::InitExtremeValues() {
	for(int i = 0; i < options_.MaxExtremeValues(); i++) {
		Gene* g = new Gene();
		for(int j = 0; j < arg_types_.size(); j++) {
			if (arg_types_[j].base_ == double_type) {
				if (arg_types_[j].is_array_) {
					ulong temp[MAX_ARRAY_SIZE];
					int sz = GetRandArraySize();
					for(int i = 0; i < sz; i++) {
						temp[i] = GetDoubleExtVal();
					}
					g->arguments_.push_back(new Argument(double_type, temp, false, true, sz));
				} else {
					ulong temp = GetDoubleExtVal();
					g->arguments_.push_back(new Argument(double_type, &temp));
				}
			} else if (arg_types_[j].base_ == int_type) {
				if (arg_types_[j].is_array_) {
					int temp[MAX_ARRAY_SIZE];
					int sz = GetRandArraySize();
					for(int i = 0; i < sz; i++) {
						temp[i] = GetIntExtVal(arg_types_[j].is_unsigned_);
					}
					g->arguments_.push_back(new Argument(int_type, temp, arg_types_[j].is_unsigned_, true, sz));
				} else {
					int temp = GetIntExtVal(arg_types_[j].is_unsigned_);
					g->arguments_.push_back(new Argument(int_type, &temp, arg_types_[j].is_unsigned_));
				}
			} else if (arg_types_[j].base_ == float_type) {
				if (arg_types_[j].is_array_) {
					uint temp[MAX_ARRAY_SIZE];
					int sz = GetRandArraySize();
					for(int i = 0; i < sz; i++) {
						temp[i] = GetFloatExtVal();
					}
					Argument* a = new Argument(float_type, temp, false, true, sz);
					g->arguments_.push_back(a);
				} else {
					uint temp = GetFloatExtVal();
					g->arguments_.push_back(new Argument(float_type, &temp));
				}
			} else if (arg_types_[j].base_ == short_type) {
				if (arg_types_[j].is_array_) {
					short temp[MAX_ARRAY_SIZE];
					int sz = GetRandArraySize();
					for(int i = 0; i < sz; i++) {
						temp[i] = GetShortExtVal(arg_types_[j].is_unsigned_);
					}
					g->arguments_.push_back(new Argument(short_type, temp, arg_types_[j].is_unsigned_, true, sz));
				} else {
					short temp = GetShortExtVal(arg_types_[j].is_unsigned_);
					g->arguments_.push_back(new Argument(short_type, &temp, arg_types_[j].is_unsigned_));
				}
			} else if (arg_types_[j].base_ == long_type) {
				if (arg_types_[j].is_array_) {
					long temp[MAX_ARRAY_SIZE];
					int sz = GetRandArraySize();
					for(int i = 0; i < sz; i++) {
						temp[i] = GetLongExtVal(arg_types_[j].is_unsigned_);
					}
					g->arguments_.push_back(new Argument(long_type, temp, arg_types_[j].is_unsigned_, true, sz));
				} else {
					long temp = GetLongExtVal(arg_types_[j].is_unsigned_);
					g->arguments_.push_back(new Argument(long_type, &temp, arg_types_[j].is_unsigned_));
				}
			// } else if (arg_types_[j].base_ == char_type) {
			// 	char temp = GetChar();
			// 	g->arguments_.push_back(new Argument(char_type, &temp));
			}
		}
		genes_.push_back(g);
		generated_tests_++;
	}
}

void Runner::InitPopulations() {
	cout << "Initialize populations..." << endl;
	cout << "Initialize extreme values..." << endl;
	InitExtremeValues();
	for(int i = 0; i < options_.MaxInitPopulations() - generated_tests_; i++) {
		Gene* g = new Gene();
		for(auto t : arg_types_) {
			if (t.base_ == double_type) {
				if (t.is_array_) {
					ulong temp[MAX_ARRAY_SIZE];
					int sz = GetRandArraySize();
					for(int i = 0; i < sz; i++) {
						temp[i] = GetDouble();
					}
					g->arguments_.push_back(new Argument(double_type, temp, false, true, sz));
				} else {
					ulong temp = GetDouble();
					g->arguments_.push_back(new Argument(double_type, &temp));
				}
			} else if (t.base_ == int_type) {
				if (t.is_array_) {
					int temp[MAX_ARRAY_SIZE];
					int sz = GetRandArraySize();
					for(int i = 0; i < sz; i++) {
						temp[i] = GetInt(t.is_unsigned_);
					}
					g->arguments_.push_back(new Argument(int_type, temp, t.is_unsigned_, true, sz));
				} else {
					int temp = GetInt(t.is_unsigned_);
					g->arguments_.push_back(new Argument(int_type, &temp, t.is_unsigned_));
				}
			} else if (t.base_ == float_type) {
				if (t.is_array_) {
					uint temp[MAX_ARRAY_SIZE];
					int sz = GetRandArraySize();
					for(int i = 0; i < sz; i++) {
						temp[i] = GetFloat();
					}
					g->arguments_.push_back(new Argument(float_type, temp, false, true, sz));
				} else {
					uint temp = GetFloat();
					g->arguments_.push_back(new Argument(float_type, &temp));
				}
			} else if (t.base_ == short_type) {
				if (t.is_array_) {
					short temp[MAX_ARRAY_SIZE];
					int sz = GetRandArraySize();
					for(int i = 0; i < sz; i++) {
						temp[i] = GetShort(t.is_unsigned_);
					}
					g->arguments_.push_back(new Argument(short_type, temp, t.is_unsigned_, true, sz));
				} else {
					short temp = GetShort(t.is_unsigned_);
					g->arguments_.push_back(new Argument(short_type, &temp, t.is_unsigned_));
				}
			} else if (t.base_ == long_type) {
				if (t.is_array_) {
					long temp[MAX_ARRAY_SIZE];
					int sz = GetRandArraySize();
					for(int i = 0; i < sz; i++) {
						temp[i] = GetLong(t.is_unsigned_);
					}
					g->arguments_.push_back(new Argument(long_type, temp, t.is_unsigned_, true, sz));
				} else {
					long temp = GetLong(t.is_unsigned_);
					g->arguments_.push_back(new Argument(long_type, &temp, t.is_unsigned_));
				}
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

GeneticRunner::GeneticRunner(const RunnerOptions& op, const string project) : Runner(op, project) {}

Status GeneticRunner::Start() {
	cout << "Start!" << endl;
	Status read_status = ReadFunctionMetadata();
	if (!read_status.OK()) return read_status;
	InitDoublePool();
	InitIntPool();
	InitShortPool();
	InitFloatPool();
	InitLongPool();
	InitPopulations();
	// PrintGenes();
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
			if (arg->IsArray()) {
				ulong* value = (ulong*)arg->Val();
				ulong temp[MAX_ARRAY_SIZE];
				for(int i = 0; i < arg->Length(); i++) {
					temp[i] = MutateDouble(value[i]);
				}
				new_gene->arguments_.push_back(new Argument(double_type, temp, false, true, arg->Length()));
			} else {
				ulong value = *(ulong*)arg->Val();
				ulong temp = MutateDouble(value);
				new_gene->arguments_.push_back(new Argument(double_type, &temp));
			}
		} else if (arg->type_ == int_type) {
			if (arg->IsArray()) {
				int* value = (int*)arg->Val();
				int temp[MAX_ARRAY_SIZE];
				for(int i = 0; i < arg->Length(); i++) {
					temp[i] = MutateInt(value[i], arg->IsUnsigned());
				}
				new_gene->arguments_.push_back(new Argument(int_type, temp, arg->IsUnsigned(), true, arg->Length()));
			} else {
				int value = *(int*)arg->Val();
				int temp = MutateInt(value, arg->IsUnsigned());
				new_gene->arguments_.push_back(new Argument(int_type, &temp, arg->IsUnsigned()));
			}
		} else if (arg->type_ == float_type) {
			if (arg->IsArray()) {
				uint* value = (uint*)arg->Val();
				uint temp[MAX_ARRAY_SIZE];
				for(int i = 0; i < arg->Length(); i++) {
					temp[i] = MutateFloat(value[i]);
				}
				new_gene->arguments_.push_back(new Argument(float_type, temp, false, true, arg->Length()));
			} else {
				uint value = *(uint*)arg->Val();
				uint temp = MutateFloat(value);
				new_gene->arguments_.push_back(new Argument(float_type, &temp));
			}
		} else if (arg->type_ == short_type) {
			if (arg->IsArray()) {
				short* value = (short*)arg->Val();
				short temp[MAX_ARRAY_SIZE];
				for(int i = 0; i < arg->Length(); i++) {
					temp[i] = MutateShort(value[i], arg->IsUnsigned());
				}
				new_gene->arguments_.push_back(new Argument(short_type, temp, arg->IsUnsigned(), true, arg->Length()));
			} else {
				short value = *(short*)arg->Val();
				short temp = MutateShort(value, arg->IsUnsigned());
				new_gene->arguments_.push_back(new Argument(short_type, &temp, arg->IsUnsigned()));
			}
		} else if (arg->type_ == long_type) {
			if (arg->IsArray()) {
				long* value = (long*)arg->Val();
				long temp[MAX_ARRAY_SIZE];
				for(int i = 0; i < arg->Length(); i++) {
					temp[i] = MutateLong(value[i], arg->IsUnsigned());
				}
				new_gene->arguments_.push_back(new Argument(long_type, temp, arg->IsUnsigned(), true, arg->Length()));
			} else {
				long value = *(long*)arg->Val();
				long temp = MutateLong(value, arg->IsUnsigned());
				new_gene->arguments_.push_back(new Argument(long_type, &temp, arg->IsUnsigned()));
			}
		}
		cout << new_gene->arguments_[new_gene->arguments_.size() - 1]->DebugString() << endl;
	}
	genes_.push_back(new_gene);
	return new_gene;
}

ulong GeneticRunner::MutateDouble(ulong value) {
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
	return CreateDouble(sign, expt, MutateDoubleSignificand(value));
}

uint GeneticRunner::MutateFloat(uint value) {
	int expt = (value & Exp32) >> 23;
	if (expt >= 122 && expt <= 134) {
		expt = expt + rand() % 2 - 1;
	} else {
		expt = expt + rand() % 20 - 10;
		if (expt < 0) {
			expt = 0;
		}
		if (expt > 254) {
			expt = 254;
		}
	}
	int sign = (value & Sign32) >> 31;
	if (rand() % 10 == 0) sign = 1 - sign;
	return CreateFloat(sign, expt, MutateFloatSignificand(value));
}

int GeneticRunner::MutateInt(int value, bool is_unsigned = false) { // TODO: overflow and underflow
	int gap = value / 2;
	if (gap == 0) {
		gap = 1 << 4;
	}
	int new_value = value + rand() % gap - (gap / 2);
	if (!is_unsigned && rand() % 10 == 0) new_value = -new_value;
	return new_value;
}

short GeneticRunner::MutateShort(short value, bool is_unsigned = false) { // TODO: overflow and underflow
	short gap = value / 2;
	if (gap == 0) {
		gap = 1 << 4;
	}
	short new_value = value + rand() % gap - (gap / 2);
	if (!is_unsigned && rand() % 10 == 0) new_value = -new_value;
	return new_value;
}

long GeneticRunner::MutateLong(long value, bool is_unsigned = false) { // TODO: overflow and underflow
	long gap = value / 2;
	if (gap == 0) {
		gap = 1 << 4;
	}
	long new_value = value + rand() % gap - (gap / 2);
	if (!is_unsigned && rand() % 10 == 0) new_value = -new_value;
	return new_value;
}

ulong GeneticRunner::MutateDoubleSignificand(ulong value) {
	ulong mask = 1L;
	for(int i = 0; i < 52; i++) {
		if (rand() % 10 == 0) value = value ^ mask;
		mask <<= 1;
	}
	return value;
}

uint GeneticRunner::MutateFloatSignificand(uint value) {
	uint mask = 1;
	for(int i = 0; i < 23; i++) {
		if (rand() % 10 == 0) value = value ^ mask;
		mask <<= 1;
	}
	return value;
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
	dexp_pool_.clear();
	dexp_pool_.push_back(0);
	dexp_pool_.push_back(2046);

	for (int i = 50; i < 980; i += 100) { // 10
		dexp_pool_.push_back(i + rand()%100 - 50);
	}

	for (int i = 980; i < 1070; i += 5) { // 18
		dexp_pool_.push_back(i + rand() % 10 - 5);
	}

	for (int i = 1070; i < 1997; i += 100) { // 10
		dexp_pool_.push_back(i + rand() % 100 - 50);
	}
}

void GeneticRunner::InitFloatPool() {
	cout << "Initialize float pool..." << endl;
	fexp_pool_.clear();
	fexp_pool_.push_back(0);
	fexp_pool_.push_back(254);

	for (int i = 5; i < 122; i += 10) { // 12
		fexp_pool_.push_back(i + rand()%10 - 5);
	}

	for (int i = 122; i < 134; i += 1) { // 12 * 2 = 24
		fexp_pool_.push_back(i);
		fexp_pool_.push_back(i);
	}

	for (int i = 134; i < 254; i += 10) { // 12
		fexp_pool_.push_back(i + rand() % 10 - 5);
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
		// int_pool_.push_back(-(1 << i));
	}
	int_pool_.push_back(0);
}

void GeneticRunner::InitShortPool() {
	cout << "Initialize short pool..." << endl;
	short_pool_.clear();
	for(int i = 0; i < 16; i++) {
		short_pool_.push_back((1 << i));
		// short_pool_.push_back(-(1 << i));
	}
	short_pool_.push_back(0);
}

void GeneticRunner::InitLongPool() {
	cout << "Initialize long pool..." << endl;
	long_pool_.clear();
	for(int i = 0; i < 64; i++) {
		long_pool_.push_back((1L << i));
		// long_pool_.push_back(-(1L << i));
	}
	long_pool_.push_back(0);
}

ulong GeneticRunner::GetDouble() {
	int sign, expt;
	sign = rand() % 2;
	expt = dexp_pool_[rand() % dexp_pool_.size()];
	if (expt >= 980 && expt < 1070) {
		expt += (rand() % 20 - 10);
	} else {
		expt += (rand() % 200 - 100);
	}
	if (expt > 2046) expt = 2046;
	if (expt < 0) expt = 0;
	return CreateDouble(sign, expt);
}

uint GeneticRunner::GetFloat() {
	int sign, expt;
	sign = rand() % 2;
	expt = fexp_pool_[rand() % fexp_pool_.size()];
	if (expt >= 122 && expt < 134) {
		expt += (rand() % 2 - 1);
	} else {
		expt += (rand() % 20 - 10);
	}
	if (expt > 254) expt = 254;
	if (expt < 0) expt = 0;
	return CreateFloat(sign, expt);
}

int GeneticRunner::GetInt(bool is_unsigned = false) {
	int value = int_pool_[rand() % int_pool_.size()];
	if (abs(value) > 1) {
		int gap = value / 2;
		value = value + rand() % gap - (gap / 2);
	}
	if (!is_unsigned && rand() % 2 == 1) return -value;
	else return value;
}

short GeneticRunner::GetShort(bool is_unsigned = false) {
	short value = short_pool_[rand() % short_pool_.size()];
	if (abs(value) > 1) {
		short gap = value / 2;
		value = value + rand() % gap - (gap / 2);
	}
	if (!is_unsigned && rand() % 2 == 1) return -value;
	else return value;
}

long GeneticRunner::GetLong(bool is_unsigned = false) {
	long value = short_pool_[rand() % short_pool_.size()];
	if (abs(value) > 1L) {
		long gap = value / 2;
		value = value + rand() % gap - (gap / 2);
	}
	if (!is_unsigned && rand() % 2 == 1) return -value;
	else return value;
}

char GeneticRunner::GetChar() {
	return 'a' + rand() % 26;
}

RandomRunner::RandomRunner(const RunnerOptions& op, const string project) : Runner(op, project) {}

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

uint RandomRunner::GetFloat() {
	return RandUint();
}

int RandomRunner::GetInt(bool is_unsigned = false) {
	return RandUint();
}

short RandomRunner::GetShort(bool is_unsigned = false) {
	short value = rand() % ((1 << 15) - 1);
	if (rand() % 2 == 1) return -value;
	else return value;
}

long RandomRunner::GetLong(bool is_unsigned = false) {
	return RandUlong();
}

char RandomRunner::GetChar() {
	return 'a' + rand() % 26;
}

StdGeneticRunner::StdGeneticRunner(const RunnerOptions& op, const string project) : GeneticRunner(op, project) {}

Status StdGeneticRunner::Start() {
	cout << "Start!" << endl;
	Status read_status = ReadFunctionMetadata();
	if (!read_status.OK()) return read_status;
	InitPopulations();
	// PrintGenes();
	for(auto gene: genes_) {
		TestGene(gene);
	}
	Evolution();
}

ulong StdGeneticRunner::GetDouble() {
	return RandUlong();
}

uint StdGeneticRunner::GetFloat() {
	return RandUint();
}

int StdGeneticRunner::GetInt(bool is_unsigned = false) {
	return RandUint();
}

short StdGeneticRunner::GetShort(bool is_unsigned = false) {
	short value = rand() % ((1 << 15) - 1);
	if (rand() % 2 == 1) return -value;
	else return value;
}

long StdGeneticRunner::GetLong(bool is_unsigned = false) {
	return RandUlong();
}

char StdGeneticRunner::GetChar() {
	return 'a' + rand() % 26;
}

ulong StdGeneticRunner::MutateDouble(ulong value) {
	int expt = (value & Exp64) >> 52;
	expt = expt + rand() % 100 - 50;
	if (expt < 0) {
		expt = 0;
	}
	if (expt > 2046) {
		expt = 2046;
	}
	int sign = (value & Sign64) >> 63;
	if (rand() % 10 == 0) sign = 1 - sign;
	return CreateDouble(sign, expt, MutateDoubleSignificand(value));
}

uint StdGeneticRunner::MutateFloat(uint value) {
	int expt = (value & Exp32) >> 23;
	expt = expt + rand() % 20 - 10;
	if (expt < 0) {
		expt = 0;
	}
	if (expt > 254) {
		expt = 254;
	}
	int sign = (value & Sign32) >> 31;
	if (rand() % 10 == 0) sign = 1 - sign;
	return CreateFloat(sign, expt, MutateFloatSignificand(value));
}

// InputRunner::InputRunner(const RunnerOptions& op) : Runner(op) {}

// Status InputRunner::Start() {
// 	cout << "Start!" << endl;
// 	Status read_status = ReadFunctionMetadata();
// 	if (!read_status.OK()) return read_status;
// 	InitDoublePool();
// 	InitIntPool();
// 	InitPopulations();
// 	DumpPopulations();
// }

// void InputRunner::InitDoublePool() {
// 	cout << "Initialize double pool..." << endl;
// 	exp_pool_.clear();
// 	exp_pool_.push_back(0);
// 	exp_pool_.push_back(2046);

// 	for (int i = 50; i < 980; i += 100) {
// 		exp_pool_.push_back(i + rand()%100 - 50);
// 	}

// 	for (int i = 980; i < 1070; i += 5) {
// 		exp_pool_.push_back(i + rand() % 10 - 5);
// 	}

// 	for (int i = 1070; i < 1997; i += 100) {
// 		exp_pool_.push_back(i + rand() % 100 - 50);
// 	}
// }

// void InputRunner::InitIntPool() {
// 	cout << "Initialize int pool..." << endl;
// 	int_pool_.clear();
// 	for(int i = 0; i < 32; i++) {
// 		int_pool_.push_back((1 << i));
// 		int_pool_.push_back(-(1 << i));
// 	}
// }


// ulong InputRunner::GetDouble() {
// 	int sign, expt;
// 	sign = rand() % 2;
// 	expt = exp_pool_[rand() % exp_pool_.size()];
// 	if (expt >= 980 && expt < 1070) {
// 		expt += (rand() % 20 - 10);
// 	} else {
// 		expt += (rand() % 200 - 100);
// 	}
// 	if (expt > 2046) expt = 2046;
// 	if (expt < 0) expt = 0;
// 	return CreateDouble(sign, expt);
// }

// int InputRunner::GetInt() {
// 	int value = int_pool_[rand() % int_pool_.size()];
// 	int new_value = value + rand() % value - (value / 2);
// 	return new_value;
// }

// void InputRunner::DumpPopulations() {
// 	string outputFile = "input/" + options_.Filename() + "_input.txt";
// 	FILE * output = fopen(outputFile.c_str(), "w");
// 	for(auto gene : genes_) {
// 		for(auto arg : gene->arguments_) {
// 			if (arg->type_ == double_type) {
// 				double dv = *(double*)arg->Val();
// 				fprintf(output, "%.100le ", dv);
// 			} else if (arg->type_ == int_type) {
// 				int iv = *(int*)arg->Val();
// 				fprintf(output, "%d ", iv);
// 			}
// 		}
// 		fprintf(output, "\n");
// 	}
// 	fclose(output);
// }

}