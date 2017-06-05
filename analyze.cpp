#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>

using namespace std;

int main(int argc, char const *argv[])
{
	if (argc != 2) {
		cerr << "Wrong number of arguments!" << endl;
		return 1;
	}
	cout << "Analyze " << argv[1] << endl;
	ifstream in(argv[1]);
	string temp;
	in >> temp >> temp >> temp;
	int total = 0;
	int gbest, rbest, sbest;
	gbest = rbest = sbest = 0;
	int gbestRig, rbestRig, sbestRig;
	gbestRig = rbestRig = sbestRig = 0;
	int equal = 0;
	getline(in, temp);
	while(getline(in, temp)) {
		stringstream ssin(temp);
		ssin >> temp;
		ssin >> temp;
		double geneticResult, randomResult, stdResult;
		ssin >> geneticResult >> temp >> randomResult >> temp >> stdResult;
		total++;
		if (geneticResult >= randomResult && geneticResult >= stdResult) {
			gbest++;
		} 
		if (randomResult >= geneticResult && randomResult >= stdResult) {
			rbest++;
		}
		if (stdResult >= randomResult && stdResult >= geneticResult) {
			sbest++;
		}
		if (geneticResult > randomResult && geneticResult > stdResult) {
			gbestRig++;
		}
		if (randomResult > geneticResult && randomResult > stdResult) {
			rbestRig++;
		}
		if (stdResult > geneticResult && stdResult > randomResult) {
			sbestRig++;
		}
		if (geneticResult == randomResult && randomResult == stdResult) {
			equal++;
		}
	}
	in.close();
	string s(argv[1]);
	s = s + ".analyze";
	ofstream out(s.c_str());
	out << "TotalCount: " << total << endl;
	out << "BestCount:" << endl;
	out << " LSGA: " << gbest<< endl;
	out << " Random: " << rbest << endl;
	out << " StdGenetic: " << sbest << endl;
	out << "BestPercentage:" << endl;
	out << " LSGA: " << gbest * 1.0 / total << endl;
	out << " Random: " << rbest * 1.0 / total << endl;
	out << " StdGenetic: " << sbest * 1.0 / total << endl;
	out << "EqualCount: " << equal << endl;
	out << "EqualPercentage: " << equal * 1.0 / total << endl;
	out << "Rigorous Version:" << endl;
	out << "BestCount:" << endl;
	out << " LSGA: " << gbestRig<< endl;
	out << " Random: " << rbestRig << endl;
	out << " StdGenetic: " << sbestRig << endl;
	out << "BestPercentage:" << endl;
	out << " LSGA: " << gbestRig * 1.0 / total << endl;
	out << " Random: " << rbestRig * 1.0 / total << endl;
	out << " StdGenetic: " << sbestRig * 1.0 / total << endl;
	out.close();
	return 0;
}