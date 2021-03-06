#include "genetic.h"
#include "status.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <time.h>

using namespace std;
using namespace genetic;

int main(int argc, char const *argv[])
{
	if (argc != 3) {
		cerr << "Invalid number of arguments." << endl;
		return 0;
	}
	time_t start, end;
	time(&start);
	RunnerOptions options;
	options.SetFilename(string(argv[1]));
	GeneticRunner g_runner(options, string(argv[2]));
	Status status = g_runner.Start();
	if (status.OK()) {
		Gene g = g_runner.GetBestGene();
		time(&end);
		char filename[100] = "geneticlog/";
		strcat(filename, argv[2]);
		strcat(filename , "/");
		strcat(filename, argv[1]);
		strcat(filename, ".log");
		ofstream out(filename);
		for(auto arg: g.arguments_) {
			out << arg->DebugString() << " ";
		}
		out << endl;
		out << difftime(end, start) << endl;
		out << g.frac_ << "e" << g.exp_ << endl;
		out.close();
	} else {
		cerr << status.ErrorMessage() << endl;
	}

	time(&start);
	StdGeneticRunner std_g_runner(options, string(argv[2]));
	status = std_g_runner.Start();
	if (status.OK()) {
		Gene g = std_g_runner.GetBestGene();
		time(&end);
		char filename[100] = "stdgeneticlog/";
		strcat(filename, argv[2]);
		strcat(filename , "/");
		strcat(filename, argv[1]);
		strcat(filename, ".log");
		ofstream out(filename);
		for(auto arg: g.arguments_) {
			out << arg->DebugString() << " ";
		}
		out << endl;
		out << difftime(end, start) << endl;
		out << g.frac_ << "e" << g.exp_ << endl;
		out.close();
	}

	time(&start);
	options.SetMaxInitPopulations(options.MaxPopulations());
	RandomRunner r_runner(options, string(argv[2]));
	status = r_runner.Start();
	if (status.OK()) {
		Gene g = r_runner.GetBestGene();
		time(&end);
		char filename[100] = "randomlog/";
		strcat(filename, argv[2]);
		strcat(filename , "/");
		strcat(filename, argv[1]);
		strcat(filename, ".log");
		ofstream out(filename);
		for(auto arg: g.arguments_) {
			out << arg->DebugString() << " ";
		}
		out << endl;
		out << difftime(end, start) << endl;
		out << g.frac_ << "e" << g.exp_ << endl;
		out.close();
	}
	return 0;
}