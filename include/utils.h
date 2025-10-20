#if !defined UTILS_H
#define UTILS_H

enum class ValidationLevel
{
	none = 0,
	basic = 1,
	light = 2,
	full = 3
};

void setValidationLevel(ValidationLevel level);
ValidationLevel getValidationLevel();

bool getAllowResizeStack();
void setAllowResizeStack(bool allow);

#endif