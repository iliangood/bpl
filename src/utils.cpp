#include "utils.h"

ValidationLevel validationLevel_ = ValidationLevel::basic; // Default

bool allowResizeStack_ = false; // because the current implementation is very insecure, if it works at all

void setValidationLevel(ValidationLevel level)
{
	validationLevel_ = level;
}

ValidationLevel getValidationLevel()
{
	return validationLevel_;
}

bool getAllowResizeStack()
{
	return allowResizeStack_;
}

void setAllowResizeStack(bool allow)
{
	allowResizeStack_ = allow;
}