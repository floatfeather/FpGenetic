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
	int MaxExtremeValues() const;
	void SetFilename(const string&);
	void SetMaxInitPopulations(int);
	void SetExecutionTimeout(int);
	void SetMaxPopulations(int);
	void SetMaxExtremeValues(int);
	RunnerOptions& operator=(const RunnerOptions& rhs);

private:
	string filename_;
	int max_init_populations_;
	int execution_timeout_;
	int max_populations_;
	int max_extreme_values_;
};

class Runner {
public:
	Runner(const RunnerOptions& op, const string project);
	virtual ~Runner();
	virtual Status Start() = 0;
	virtual unsigned long int GetDouble() = 0;
	virtual int GetInt(bool) = 0;
	virtual unsigned int GetFloat() = 0;
	virtual short GetShort(bool) = 0;
	virtual long GetLong(bool) = 0;
	virtual char GetChar() = 0;

	virtual Status ReadFunctionMetadata();
	virtual void UpdateMaxError(Gene* gene);
	virtual void GetRelativeError(Gene* gene);
	virtual void TestGene(Gene* gene);
	virtual void RunFPDebug(const Gene& gene);
	virtual void InitExtremeValues();
	virtual Gene GetBestGene();
	virtual void InitPopulations();
	virtual ulong RandUlong();
	virtual unsigned int RandUint();
	virtual void PrintGenes();
	virtual unsigned long int CreateDouble(int sign, int expt);
	virtual unsigned long int CreateDouble(int sign, int expt, unsigned long int ufrac);
	virtual unsigned int CreateFloat(int sign, int expt);
	virtual unsigned int CreateFloat(int sign, int expt, unsigned int ufrac);
	virtual ulong GetDoubleExtVal();
	virtual unsigned int GetFloatExtVal();
	virtual int GetIntExtVal(bool);
	virtual short GetShortExtVal(bool);
	virtual long GetLongExtVal(bool);

	RunnerOptions options_;
	vector<Gene*> genes_;
	vector<Type> arg_types_;
	int generated_tests_;
	int pipefd_[2];
	Gene* best_gene_;
	vector<Stage> stages_;
	string project_;
};

class GeneticRunner : protected Runner {
public:
	GeneticRunner(const RunnerOptions& op, const string project);
	Status Start();
	using Runner::GetBestGene;

protected:
	unsigned long int GetDouble();
	int GetInt(bool);
	unsigned int GetFloat();
	short GetShort(bool);
	long GetLong(bool);
	char GetChar();
	void Evolution();
	Gene* Mutate(const Gene& gene);
	void GroupGenes();
	Gene* RandomPick();
	void InitDoublePool();
	void InitIntPool();
	void InitFloatPool();
	void InitLongPool();
	void InitShortPool();
	unsigned long int MutateDouble(unsigned long int);
	unsigned int MutateFloat(unsigned int);
	int MutateInt(int, bool);
	short MutateShort(short, bool);
	long MutateLong(long, bool);
	ulong MutateDoubleSignificand(ulong);
	unsigned int MutateFloatSignificand(unsigned int);

private:
	vector<int> dexp_pool_;
	vector<int> int_pool_;
	vector<int> fexp_pool_;
	vector<short> short_pool_;
	vector<long> long_pool_;
};

class RandomRunner : protected Runner {
public:
	RandomRunner(const RunnerOptions& op, const string project);
	Status Start();
	using Runner::GetBestGene;

protected:
	unsigned long int GetDouble();
	int GetInt(bool);
	unsigned int GetFloat();
	short GetShort(bool);
	long GetLong(bool);
	char GetChar();
};

class StdGeneticRunner : protected GeneticRunner {
public:
	StdGeneticRunner(const RunnerOptions& op, const string project);
	Status Start();
	using GeneticRunner::GetBestGene;

protected:
	unsigned long int GetDouble();
	int GetInt(bool);
	unsigned int GetFloat();
	short GetShort(bool);
	long GetLong(bool);
	char GetChar();
	unsigned long int MutateDouble(unsigned long int);
	unsigned int MutateFloat(unsigned int);
};

// class InputRunner : private Runner {
// public:
// 	InputRunner(const RunnerOptions& op);
// 	Status Start();

// private:
// 	void InitDoublePool();
// 	void InitIntPool();
// 	unsigned long int GetDouble();
// 	int GetInt(bool);
// 	void DumpPopulations();

// 	vector<int> exp_pool_;
// 	vector<int> int_pool_;
// };

}

#endif // WORKSPACE_GENETIC_GENETIC_H