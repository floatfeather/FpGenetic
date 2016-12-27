#ifndef WORKSPACE_GENETIC_GENETIC_H
#define WORKSPACE_GENETIC_GENETIC_H

#include "argument.h"
#include "status.h"
#include <vector>

namespace genetic {

struct Gene {
public:
	vector<Argument*> arguments_;
	double frac_;
	int exp_;
};

struct Stage {
public:
	Gene* max_gene_;
	vector<Gene*> elements_;
	double weight_;
};

class RunnerOptions {
public:
	RunnerOptions();
	string Filename() const;
	int MaxInitPopulations() const;
	int ExecutionTimeout() const;
	int MaxPopulations() const;
	void SetFilename(const string&);
	void SetMaxInitPopulations(int);
	void SetExecutionTimeout(int);
	void SetMaxPopulations(int);
	RunnerOptions& operator=(const RunnerOptions& rhs);

private:
	string filename_;
	int max_init_populations_;
	int execution_timeout_;
	int max_populations_;
};

class Runner {
public:
	Runner(const RunnerOptions& op);
	virtual ~Runner();
	virtual Status Start() = 0;
	virtual unsigned long int GetDouble() = 0;
	virtual int GetInt() = 0;

	virtual Status ReadFunctionMetadata();
	virtual void UpdateMaxError(Gene* gene);
	virtual void GetRelativeError(Gene* gene);
	virtual void TestGene(Gene* gene);
	virtual void RunFPDebug(const Gene& gene);
	virtual void InitExtremeValues(int i, vector<int>* values);
	virtual Gene GetBestGene();
	virtual void InitPopulations();
	virtual ulong RandUlong();
	virtual void PrintGenes();
	virtual unsigned long int CreateDouble(int sign, int expt);

	RunnerOptions options_;
	int double_cnt_;
	int int_cnt_;
	vector<Gene*> genes_;
	vector<TypeName> arg_types_;
	int generated_tests_;
	int pipefd_[2];
	Gene* best_gene_;
	vector<Stage> stages_;
};

class GeneticRunner : private Runner {
public:
	GeneticRunner(const RunnerOptions& op);
	Status Start();
	using Runner::GetBestGene;

private:
	unsigned long int GetDouble();
	int GetInt();
	void Evolution();
	Gene* Mutate(const Gene& gene);
	void GroupGenes();
	Gene* RandomPick();
	void InitDoublePool();
	void InitIntPool();

	vector<int> exp_pool_;
	vector<int> int_pool_;
};

class RandomRunner : private Runner {
public:
	RandomRunner(const RunnerOptions& op);
	Status Start();
	using Runner::GetBestGene;

private:
	unsigned long int GetDouble();
	int GetInt();
};

class InputRunner : private Runner {
public:
	InputRunner(const RunnerOptions& op);
	Status Start();

private:
	void InitDoublePool();
	void InitIntPool();
	unsigned long int GetDouble();
	int GetInt();
	void DumpPopulations();

	vector<int> exp_pool_;
	vector<int> int_pool_;
};

}

#endif // WORKSPACE_GENETIC_GENETIC_H