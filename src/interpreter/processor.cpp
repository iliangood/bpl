#include "interpreter/processor.h"


std::optional<std::variant<int, std::string>> Processor::execute(Instruction instruction)
{
	if(finished())
	{
		return std::nullopt;
	}
	if(instruction.opCode() == OpCode::end_)
	{
		finished_ = true;
		if(instruction.arguments().size() == 0)
			return std::nullopt;
		if(instruction.arguments().size() == 1)
		{
			if(auto retVal = std::get_if<int>(&instruction.arguments()[0]))
				return *retVal;
		}
		throw std::runtime_error("Invalid end instruction");
	}	

}

bool Processor::finished() { return finished_; }