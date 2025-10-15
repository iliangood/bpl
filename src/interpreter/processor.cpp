#include "interpreter/processor.h"


std::optional<std::variant<int, std::string>> Processor::execute(Instruction instruction)
{
	if(finished())
	if(instruction.opCode() == OpCode::end)
	{
		finished_ = true;
		return std::nullopt;
	}	

}

bool Processor::finished() { return finished_; }