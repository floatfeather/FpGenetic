#include "argument.h"
#include <stdio.h>

Argument::Argument(TypeName t, void* value) {
	type_ = t;
	if (t == double_type) {
		double_val_ = *(unsigned long int*)value;
	} else if (t == int_type) {
		int_val_ = *(int*)value;
	}
}

const void* Argument::Val() {
	if (type_ == double_type) {
		return &double_val_; 
	} else if (type_ == int_type) {
		return &int_val_;
	}
}

void Argument::ToString(char * buf) {
	if (type_ == double_type) {
		sprintf(buf, "%lX", double_val_);
	} else if (type_ == int_type) {
		sprintf(buf, "%d", int_val_);
	}
}

string Argument::DebugString() {
	char buf[100];
	if (type_ == double_type) {
		double temp = *(double*)&double_val_;
		sprintf(buf, "%.10le", temp);
	} else if (type_ == int_type) {
		sprintf(buf, "%d", int_val_);
	}
	return string(buf);
}