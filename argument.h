#ifndef WORKSPACE_ARGUMENT_H
#define WORKSPACE_ARGUMENT_H

#define MAX_ARRAY_SIZE 5

#include <string>

using namespace std;

enum TypeName {
	double_type = 0,
	float_type = 1,
	long_type = 2,
	int_type = 3,
	short_type = 4,
	// char_type = 5
};

struct Type {
	TypeName base_;
	bool is_unsigned_;
	bool is_array_;
	Type() {
		is_unsigned_ = false;
		is_array_ = false;
	}
};

class Argument {
public:
	Argument(TypeName t, void* value, bool is_unsigned = false, bool is_array = false, int array_size = 0);
	TypeName type_;
	void ToString(char *);
	string DebugString();
	const void* Val();
	bool IsUnsigned();
	bool IsArray();
	int Length();

private:
	unsigned long int double_val_;
	unsigned int float_val_;
	int int_val_;
	long long_val_;
	short short_val_;
	// char char_val_;
	bool is_unsigned_;
	bool is_array_;
	unsigned long int double_arr_val_[MAX_ARRAY_SIZE];
	unsigned int float_arr_val_[MAX_ARRAY_SIZE];
	int int_arr_val_[MAX_ARRAY_SIZE];
	long long_arr_val_[MAX_ARRAY_SIZE];
	short short_arr_val_[MAX_ARRAY_SIZE];
	// char* char_arr_val_;
	int array_size_;
};

static int GetRandArraySize() {
	return rand() % MAX_ARRAY_SIZE + 1;
}
#endif