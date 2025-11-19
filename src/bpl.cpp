#include <iostream>

#include "interpreter/processor.h"

int main(/*int argc, char** argv*/)
{
	setValidationLevel(ValidationLevel::basic);
	Processor proc(1 << 20);

	return 0;
}