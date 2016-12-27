#ifndef WORKSPACE_ARGUMENT_H
#define WORKSPACE_ARGUMENT_H

#include <string>

using namespace std;

enum TypeName {
	double_type = 0,
	int_type = 1
};

class Argument {
public:
	Argument(TypeName t, void * value);
	TypeName type_;
	void ToString(char *);
	string DebugString();
	const void* Val();

private:
	unsigned long int double_val_;
	int int_val_;
};
#endif