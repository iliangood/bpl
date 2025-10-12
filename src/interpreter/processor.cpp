#include "interpreter/processor.h"


void Processor::execute(Instruction instruction)
{
	if(finished())
	if(instruction.opCode() == OpCode::end)
	{
		finished_ = true;
		return;
	}	

}

bool Processor::finished() { return finished_; }