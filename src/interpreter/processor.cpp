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



Processor::Processor(const std::vector<Instruction>& program, size_t stackSize) : programm_(program),
stack_(stackSize), finished_(false), returningFromFunction_(false)
{
	baseTypes_.resize(BaseTypeId::countOfBaseTypes);
	baseTypes_[BaseTypeId::int64_] = BaseType("int64_t", sizeof(int64_t));
	baseTypes_[BaseTypeId::uint64_] = BaseType("uint64_t", sizeof(uint64_t));
	baseTypes_[BaseTypeId::char_] = BaseType("char", sizeof(char));
	baseTypes_[BaseTypeId::size_] = BaseType("size", sizeof(size_t));
	baseTypes_[BaseTypeId::double_] = BaseType("double", sizeof(double));
	baseTypes_[BaseTypeId::void_] = BaseType("void", 0);
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

std::optional<int64_t> Processor::end_(Instruction&& instruction)
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

std::optional<int64_t> Processor::call_(Instruction&& instruction)
{
	if(instruction.arguments().size() < 1)
		throw std::runtime_error("Invalid call instruction");
	std::vector<Instruction>* funcBody;
	size_t offset = 0;
	if(std::holds_alternative<TypeVariant>(instruction.arguments()[0]))
	{
		if(!isFunctionType(std::get<TypeVariant>(instruction.arguments()[0])))
			throw std::runtime_error("Invalid call instruction argument");
		if(!std::get<FunctionType>(std::get<TypeVariant>(instruction.arguments()[0])).isValid())
			throw std::runtime_error("Invalid function type in call instruction");
		if(!std::holds_alternative<StackIndex>(instruction.arguments()[1]))
			throw std::runtime_error("Invalid call instruction argument");
		StackIndex funcIndex = std::get<StackIndex>(instruction.arguments()[1]);
		if(!funcIndex.isValid())
			throw std::runtime_error("Invalid StackIndex in call instruction argument");
		Element funcElement = stack_.element(funcIndex.index());
		if(!isFunctionType(funcElement.type()))
			throw std::runtime_error("StackIndex in call instruction does not point to a FunctionType");
		funcBody = reinterpret_cast<std::vector<Instruction>*>(stack_.at(funcElement.pos()));
		offset = 2;
	}
	else if(std::holds_alternative<Function>(instruction.arguments()[0]))
	{
		funcBody = &std::get<Function>(instruction.arguments()[0]).body();
		offset = 1;
	}
	else
		throw std::runtime_error("Invalid call instruction argument");
		
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

	functionEntry();
	if(func.argumentsTypes().size() != instruction.arguments().size() - offset)
		throw std::runtime_error("Invalid number of call arguments");
	for(size_t i = offset; i < instruction.arguments().size(); ++i)
	{
		if(std::holds_alternative<StackIndex>(instruction.arguments()[i]))
		{
			StackIndex index = std::get<StackIndex>(instruction.arguments()[i]);
			if(!index.isValid())
				throw std::runtime_error("Invalid StackIndex in call instruction argument");
			if(!isTypeCompatible(stack_.element(index.index()).type(), func.argumentsTypes()[i - offset]))
				throw std::runtime_error("Incompatible type for call instruction argument");
			stack_.push(stack_.element(index.index()));
		}
		else if(std::holds_alternative<int64_t>(instruction.arguments()[i]))
		{
			int64_t value = std::get<int64_t>(instruction.arguments()[i]);
			TypeVariant expectedType = func.argumentsTypes()[i - offset];
			if(!isTypeCompatible(expectedType, TypeVariant(&baseTypes_[BaseTypeId::int64_])))
				throw std::runtime_error("Incompatible type for call instruction argument");
			ElementInfo elemInfo(func.argumentNames()[i-offset], expectedType);
			uint8_t* dataPtr = stack_.push(elemInfo);
			*reinterpret_cast<int64_t*>(dataPtr) = value;
		}
		else if(std::holds_alternative<size_t>(instruction.arguments()[i]))
		{
			size_t value = std::get<size_t>(instruction.arguments()[i]);
			TypeVariant expectedType = func.argumentsTypes()[i - offset];
			if(!isTypeCompatible(expectedType, TypeVariant(&baseTypes_[BaseTypeId::size_])))
				throw std::runtime_error("Incompatible type for call instruction argument");
			ElementInfo elemInfo(func.argumentNames()[i-offset], expectedType);
			uint8_t* dataPtr = stack_.push(elemInfo);
			*reinterpret_cast<size_t*>(dataPtr) = value;
		}
		else if(std::holds_alternative<std::string>(instruction.arguments()[i]))
		{
			std::string value = std::get<std::string>(instruction.arguments()[i]);
			TypeVariant expectedType = func.argumentsTypes()[i - offset];
			if(!isTypeCompatible(expectedType, TypeVariant(&baseTypes_[BaseTypeId::char_])))
				throw std::runtime_error("Incompatible type for call instruction argument");
			ElementInfo elemInfo(func.argumentNames()[i-offset], expectedType);
			uint8_t* dataPtr = stack_.push(elemInfo);
			strcpy(reinterpret_cast<char*>(dataPtr), value.c_str());
		}
		else
		{
			throw std::runtime_error("Invalid argument type in call instruction");
		}
	}
	for(size_t i = 0; i < func.size(); ++i)
	{
		if(finished())
			break;
		if(returningFromFunction())
		{
			returningFromFunction_ = false;
			break;
		}
		execute(funcBody->at(i));
	}
	functionExit();
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
		
	}
	return std::nullopt;
}	

void Processor::notifyStackReallocation(uint8_t* new_data) //TODO:
{
	execute(Instruction(OpCode::stackRealloc_, {}));
}

bool Processor::finished() { return finished_; }