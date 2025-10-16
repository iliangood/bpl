#include "interpreter/processor.h"


std::optional<int64_t> Processor::execute(Instruction instruction)
{
	if(finished())
	{
		return std::nullopt;
	}
	if(instruction.opCode() == OpCode::end_)
	{
		finished_ = true;
		if(instruction.arguments().size() == 0)
			return 0;
		if(instruction.arguments().size() == 1)
		{
			if(std::holds_alternative<int64_t>(instruction.arguments()[0]))
				return std::get<int64_t>(instruction.arguments()[0]);
		}
		throw std::runtime_error("Invalid end instruction");
	}
	if(instruction.opCode() == OpCode::call_)
	{
		if(instruction.arguments().size() < 1)
			throw std::runtime_error("Invalid call instruction");
		for(size_t i = 1; i < instruction.arguments().size(); ++i)
		{
			if(std::holds_alternative<size_t>(instruction.arguments()[i]))
			{
				size_t index = std::get<size_t>(instruction.arguments()[i]);
				if(index >= stack_.size())
					throw std::runtime_error("Invalid call argument index");
				stack_.push(stack_.element(index));
				
			}
			else
				throw std::runtime_error("Invalid call instruction argument");
		}
	}
	return std::nullopt;
}	

bool Processor::finished() { return finished_; }