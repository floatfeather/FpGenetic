#include "argument.h"
#include <stdio.h>
#include <typeinfo>
#include <stdlib.h>
#include <string.h>

using namespace std;

Argument::Argument(TypeName t, void* value, bool is_unsigned, bool is_array, int array_size) {
	type_ = t;
	is_unsigned_ = is_unsigned;
	is_array_ = is_array;
	array_size_ = array_size;
	if (array_size > MAX_ARRAY_SIZE) {
		printf("Array size should not be larger than %d.\n", MAX_ARRAY_SIZE);
		exit(1);
	}
	if (!is_array_) {
		if (t == double_type) {
			double_val_ = *(unsigned long int*)value;
		} else if (t == int_type) {
			int_val_ = *(int*)value;
		} else if (t == float_type) {
			float_val_ = *(unsigned int*)value;
		} else if (t == short_type) {
			short_val_ = *(short*)value;
		} else if (t == long_type) {
			long_val_ = *(long*)value;
		}
	} else {
		if (t == double_type) {
			unsigned long int* temp = (unsigned long int*)value;
			for(int i = 0; i < array_size_; i++) {
				double_arr_val_[i] = temp[i];
			}
		} else if (t == int_type) {
			int* temp = (int*)value;
			for(int i = 0; i < array_size_; i++) {
				int_arr_val_[i] = temp[i];
			}
		} else if (t == float_type) {
			unsigned int* temp = (unsigned int*)value;
			for(int i = 0; i < array_size_; i++) {
				float_arr_val_[i] = temp[i];
			}
		} else if (t == short_type) {
			short* temp = (short*)value;
			for(int i = 0; i < array_size_; i++) {
				short_arr_val_[i] = temp[i];
			}
		} else if (t == long_type) {
			long* temp = (long*)value;
			for(int i = 0; i < array_size_; i++) {
				long_arr_val_[i] = temp[i];
			}
		}
	}
}

const void* Argument::Val() {
	if (!is_array_) {
		if (type_ == double_type) {
			return &double_val_; 
		} else if (type_ == int_type) {
			return &int_val_;
		} else if (type_ == float_type) {
			return &float_val_;
		} else if (type_ == short_type) {
			return &short_val_;
		} else if (type_ == long_type) {
			return &long_val_;
		}
	} else {
		if (type_ == double_type) {
			return double_arr_val_; 
		} else if (type_ == int_type) {
			return int_arr_val_;
		} else if (type_ == float_type) {
			return float_arr_val_;
		} else if (type_ == short_type) {
			return short_arr_val_;
		} else if (type_ == long_type) {
			return long_arr_val_;
		}
	}
}

void Argument::ToString(char * buf) {
	buf[0] = 0;
	if (!is_array_) {
		if (type_ == double_type) {
			sprintf(buf, "%lX", double_val_);
		} else if (type_ == int_type) {
			if (is_unsigned_) {
				sprintf(buf, "%u", int_val_);
			} else {
				sprintf(buf, "%d", int_val_);
			}
		} else if (type_ == float_type) {
			sprintf(buf, "%X", float_val_);
		} else if (type_ == short_type) {
			if (is_unsigned_) {
				sprintf(buf, "%hu", short_val_);
			} else {
				sprintf(buf, "%hd", short_val_);
			}
		} else if (type_ == long_type) {
			if (is_unsigned_) {
				sprintf(buf, "%lu", long_val_);
			} else {
				sprintf(buf, "%ld", long_val_);
			}
		}
	} else {
		char temp[20];
		if (type_ == double_type) {
			for(int i = 0; i < array_size_; i++) {
				sprintf(temp, "%lX ", double_arr_val_[i]);
				strcat(buf, temp);
			}
		} else if (type_ == int_type) {
			for(int i = 0; i < array_size_; i++) {
				if (is_unsigned_) {
					sprintf(temp, "%u ", int_arr_val_[i]);
				} else {
					sprintf(temp, "%d ", int_arr_val_[i]);
				}
				strcat(buf, temp);
			}
		} else if (type_ == float_type) {
			for(int i = 0; i < array_size_; i++) {
				sprintf(temp, "%X ", float_arr_val_[i]);
				strcat(buf, temp);
			}
		} else if (type_ == short_type) {
			for(int i = 0; i < array_size_; i++) {
				if (is_unsigned_) {
					sprintf(temp, "%hu ", short_arr_val_[i]);
				} else {
					sprintf(temp, "%hd ", short_arr_val_[i]);
				}
				strcat(buf, temp);
			}
		} else if (type_ == long_type) {
			for(int i = 0; i < array_size_; i++) {
				if (is_unsigned_) {
					sprintf(temp, "%lu ", long_arr_val_[i]);
				} else {
					sprintf(temp, "%ld ", long_arr_val_[i]);
				}
				strcat(buf, temp);
			}
		}
	}
}

string Argument::DebugString() {
	char buf[100 * MAX_ARRAY_SIZE];
	buf[0] = 0;
	if (!is_array_) {
		if (type_ == double_type) {
			double temp = *(double*)&double_val_;
			sprintf(buf, "%.10le", temp);
		} else if (type_ == int_type) {
			if (is_unsigned_) {
				sprintf(buf, "%u", int_val_);
			} else {
				sprintf(buf, "%d", int_val_);
			}
		} else if (type_ == float_type) {
			float temp = *(float*)&float_val_;
			sprintf(buf, "%.10e", temp);
		} else if (type_ == short_type) {
			if (is_unsigned_) {
				sprintf(buf, "%hu", short_val_);
			} else {
				sprintf(buf, "%hd", short_val_);
			}
		} else if (type_ == long_type) {
			if (is_unsigned_) {
				sprintf(buf, "%lu", long_val_);
			} else {
				sprintf(buf, "%ld", long_val_);
			}
		}
	} else {
		char tbuf[20];
		if (type_ == double_type) {
			double* temp = (double*)double_arr_val_;
			for(int i = 0; i < array_size_; i++) {
				sprintf(tbuf, "%.10le ", temp[i]);
				strcat(buf, tbuf);
			}
		} else if (type_ == int_type) {
			for(int i = 0; i < array_size_; i++) {
				if (is_unsigned_) {
					sprintf(tbuf, "%u ", int_arr_val_[i]);
				} else {
					sprintf(tbuf, "%d ", int_arr_val_[i]);
				}
				strcat(buf, tbuf);
			}
		} else if (type_ == float_type) {
			float* temp = (float*)float_arr_val_;
			for(int i = 0; i < array_size_; i++) {
				sprintf(tbuf, "%.10e ", temp[i]);
				strcat(buf, tbuf);
			}
		} else if (type_ == short_type) {
			for(int i = 0; i < array_size_; i++) {
				if (is_unsigned_) {
					sprintf(tbuf, "%hu ", short_arr_val_[i]);
				} else {
					sprintf(tbuf, "%hd ", short_arr_val_[i]);
				}
				strcat(buf, tbuf);
			}
		} else if (type_ == long_type) {
			for(int i = 0; i < array_size_; i++) {
				if (is_unsigned_) {
					sprintf(tbuf, "%lu ", long_arr_val_[i]);
				} else {
					sprintf(tbuf, "%ld ", long_arr_val_[i]);
				}
				strcat(buf, tbuf);
			}
		}
	}
	return string(buf);
}

bool Argument::IsUnsigned() {
	return is_unsigned_;
}

bool Argument::IsArray() {
	return is_array_;
}

int Argument::Length() {
	return array_size_;
}