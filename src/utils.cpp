#include "utils.h"

ValidationLevel validationLevel_ = ValidationLevel::basic; // Default

void setValidationLevel(ValidationLevel level)
{
	validationLevel_ = level;
}

ValidationLevel getValidationLevel()
{
	return validationLevel_;
}