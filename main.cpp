#include "genetic.h"
#include "status.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>

using namespace std;
using namespace genetic;

int main(int argc, char const *argv[])
{
	if (argc != 2) {
		cerr << "Invalid number of arguments." << endl;
		return 0;
	}
	RunnerOptions options;
	options.SetFilename(string(argv[1]));
	GeneticRunner g_runner(options);
	Status status = g_runner.Start();
	if (status.OK()) {
		// Gene g = g_runner.GetBestGene();
		// char filename[100] = "geneticlog/";
		// strcat(filename, argv[1]);
		// strcat(filename, ".log");
		// ofstream out(filename);
		// for(auto arg: g.arguments_) {
		// 	out << arg->DebugString() << " ";
		// }
		// out << endl;
		// out << g.frac_ << "e" << g.exp_ << endl;
		// out.close();
	} else {
		cerr << status.ErrorMessage() << endl;
	}
	// options.SetMaxInitPopulations(options.MaxPopulations());
	// RandomRunner r_runner(options);
	// status = r_runner.Start();
	// if (status.OK()) {
		// Gene g = r_runner.GetBestGene();
		// char filename[100] = "randomlog/";
		// strcat(filename, argv[1]);
		// strcat(filename, ".log");
		// ofstream out(filename);
		// for(auto arg: g.arguments_) {
		// 	out << arg->DebugString() << " ";
		// }
		// out << endl;
		// out << g.frac_ << "e" << g.exp_ << endl;
		// out.close();
	// }
	return 0;
}