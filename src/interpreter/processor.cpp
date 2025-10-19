#include "interpreter/processor.h"
#include "utils.h"

StackIndex::StackIndex(size_t index, Processor* processor, bool isGlobal) : processor_(processor) 
{
	if(processor_ == nullptr)
		throw std::invalid_argument("StackIndex::StackIndex(size_t, Processor*, bool) null Processor pointer");
	if(isGlobal)
	{
		if(index >= processor_->stack_.elementCount())
			throw std::out_of_range("StackIndex::StackIndex(size_t, Processor*, bool) global index out of range");
		index_ = index;
		return;
	}
	if(index >= processor_->stack_.elementCount() - processor_->functionStackStartPositions_.back())
		throw std::out_of_range("StackIndex::StackIndex(size_t, Processor*, bool) local index out of range");
	index_ = processor_->functionStackStartPositions_.back() + index;
}

StackIndex::StackIndex(Element element, Processor* processor) : processor_(processor) 
{
	if(processor_ == nullptr)
		throw std::invalid_argument("StackIndex::StackIndex(Element, Processor*) null Processor pointer");
	index_ = element.index();
	if(index_ >= processor_->stack_.elementCount())
		throw std::out_of_range("StackIndex::StackIndex(Element, Processor*) element index out of range");
}

bool StackIndex::isValid() const
{
	if(processor_ == nullptr)
		return false;
	if(index_ >= processor_->stack_.elementCount())
		return false;
	return true;
}



Function::Function(StackIndex index, Processor* processor) : processor_(processor)
{
	if(processor_ == nullptr)
		throw std::invalid_argument("Function::Function(StackIndex, Processor*) null Processor pointer");
	if(!index.isValid())
		throw std::invalid_argument("Function::Function(StackIndex, Processor*) invalid StackIndex");
	Element funcElement = processor_->stack_.element(index.index());
	if(!isFunctionType(funcElement.type()))
		throw std::invalid_argument("Function::Function(StackIndex, Processor*) index does not point to a FunctionType");
	type_ = std::get<FunctionType>(funcElement.type());
	body_ = *reinterpret_cast<std::vector<Instruction>*>(processor_->stack_.at(funcElement.pos()));
}



Processor::Processor(const std::vector<Instruction>& program, size_t stackSize) : programm_(program), stack_(stackSize), finished_(false)
{
	baseTypes_.push_back(BaseType("int64", sizeof(int64_t)));
	baseTypes_.push_back(BaseType("uint64", sizeof(uint64_t)));
	baseTypes_.push_back(BaseType("char", sizeof(char)));
	baseTypes_.push_back(BaseType("size_t", sizeof(size_t)));
	baseTypes_.push_back(BaseType("double", sizeof(double)));
	baseTypes_.push_back(BaseType("void", 0));
}

void Processor::functionEntry()
{
	functionStackStartPositions_.push_back(stack_.elementCount());
	stack_.newLevel();
}

void Processor::functionExit()
{
	if(functionStackStartPositions_.empty())
		throw std::runtime_error("Processor::functionExit() no function to exit from");
	functionStackStartPositions_.pop_back();
	stack_.popLevel();
}

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
		if(!std::holds_alternative<TypeVariant>(instruction.arguments()[0]))
			throw std::runtime_error("Invalid call instruction argument");
		FunctionType func;
		if(isFunctionType(std::get<TypeVariant>(instruction.arguments()[0])))
		{
			func = std::get<FunctionType>(std::get<TypeVariant>(instruction.arguments()[0]));
			if(!func.isValid())
				throw std::runtime_error("Invalid function type in call instruction");
		}
		else
			throw std::runtime_error("Invalid function type in call instruction");
		stack_.newLevel();
		functionStackStartPositions_.push_back(stack_.elementCount());
		for(size_t i = 1; i < instruction.arguments().size(); ++i)
		{
			if(std::holds_alternative<StackIndex>(instruction.arguments()[i]))
			{
				StackIndex index = std::get<StackIndex>(instruction.arguments()[i]);
				if(index.index() >= stack_.size())
					throw std::runtime_error("Invalid call argument index");
				stack_.push(stack_.element(index.index()));
			}
			else
				throw std::runtime_error("Invalid call instruction argument");
		}
	}
	return std::nullopt;
}	

void Processor::notifyStackReallocation(uint8_t* new_data) //TODO:
{
	execute(Instruction(OpCode::stackRealloc_, {}));
}

bool Processor::finished() { return finished_; }