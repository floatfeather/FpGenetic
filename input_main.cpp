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
	options.SetMaxInitPopulations(10000);
	InputRunner runner(options);
	runner.Start();
	return 0;
}