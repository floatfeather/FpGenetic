#include "status.h"

namespace genetic {
Status::Status() {
	ok_ = true;
}

Status::Status(const string& message) {
	message_ = message;
	ok_ = false;
}

string Status::ErrorMessage() {
	return message_;
}

bool Status::OK() {
	return ok_;
}
}