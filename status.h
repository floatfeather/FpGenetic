#ifndef WORKSPACE_GENETIC_ERROR_H
#define WORKSPACE_GENETIC_ERROR_H

#include <string>

using namespace std;

namespace genetic {
class Status {
public:
	Status();
	Status(const string& message);
	string ErrorMessage();
	bool OK();
private:
	string message_;
	bool ok_;
};
}
#endif