#include "interpreter/processor.h"
#include "utils.h"

StackIndex::StackIndex(size_t index, Processor* processor, const std::optional<std::vector<size_t>>& subIndexes, bool isGlobal) : processor_(processor) 
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
	subIndexes_ = subIndexes;
}

StackIndex::StackIndex(Element element, Processor* processor, const std::optional<std::vector<size_t>>& subIndexes) : processor_(processor) 
{
	if(processor_ == nullptr)
		throw std::invalid_argument("StackIndex::StackIndex(Element, Processor*) null Processor pointer");
	index_ = element.index();
	if(index_ >= processor_->stack_.elementCount())
		throw std::out_of_range("StackIndex::StackIndex(Element, Processor*) element index out of range");
	subIndexes_ = subIndexes;
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
		throw std::runtime_error("Invalid call instruction: no function specified");
	std::vector<Instruction>* functionBody = nullptr;
	FunctionType* functionType;
	if(std::holds_alternative<Function>(instruction.arguments()[0]))
	{
		Function& func = std::get<Function>(instruction.arguments()[0]); 
		if(!func.isValid())
			throw std::runtime_error("Invalid call instruction: invalid Function");
		functionBody = &func.body();
		functionType = &func.type();
	}
	else if(std::holds_alternative<StackIndex>(instruction.arguments()[0]))
	{
		StackIndex& index = std::get<StackIndex>(instruction.arguments()[0]);
		if(!index.isValid())
			throw std::runtime_error("Invalid call instruction: invalid StackIndex");
		Element funcElement = stack_.element(index.index());
		if(!isFunctionType(funcElement.type()))
			throw std::runtime_error("Invalid call instruction: StackIndex does not point to a FunctionType");
		functionType = &std::get<FunctionType>(funcElement.type());
		functionBody = reinterpret_cast<std::vector<Instruction>*>(stack_.at(funcElement.pos()));
	}
	else
	{
		throw std::runtime_error("Invalid call instruction: first argument is not Function or StackIndex");
	}
}
std::optional<int64_t> Processor::execute(Instruction instruction)
{
	if(finished())
	{
		return std::nullopt;
	}
	switch (instruction.opCode())
	{
	case OpCode::end_:
		return end_(std::move(instruction));
		break;
	case OpCode::call_:
		return call_(std::move(instruction));
		break;
	case OpCode::ret_:
		return ret_(std::move(instruction));
		break;
	case OpCode::scopeRet_:
		return scopeRet_(std::move(instruction));
		break;
	case OpCode::init_:
		return init_(std::move(instruction));
		break;
	case OpCode::mov_:
		return mov_(std::move(instruction));
		break;
	case OpCode::if_:
		return if_(std::move(instruction));
		break;
	case OpCode::while_:
		return while_(std::move(instruction));
		break;
	case OpCode::add_:
		return add_(std::move(instruction));
		break;
	case OpCode::sub_:
		return sub_(std::move(instruction));
		break;
	case OpCode::mul_:
		return mul_(std::move(instruction));
		break;
	case OpCode::div_:
		return div_(std::move(instruction));
		break;
	case OpCode::mod_:
		return mod_(std::move(instruction));
		break;
	case OpCode::and_:
		return and_(std::move(instruction));
		break;
	case OpCode::or_:
		return or_(std::move(instruction));
		break;
	case OpCode::not_:
		return not_(std::move(instruction));
		break;
	case OpCode::shl_:
		return shl_(std::move(instruction));
		break;
	case OpCode::shr_:
		return shr_(std::move(instruction));;
		break;
	case OpCode::stackRealloc_:
		return stackRealloc_(std::move(instruction));
		break;
	case OpCode::print_:
		return print_(std::move(instruction));
		break;
	case OpCode::scan_:
		return scan_(std::move(instruction));
		break;
	case OpCode::cmp_:
		return cmp_(std::move(instruction));
		break;
	default:
		break;
	}
	return std::nullopt;
}	

void Processor::notifyStackReallocation(uint8_t* /*new_data*/) // Currently does nothing
{
	execute(Instruction(OpCode::stackRealloc_, {}));
}

bool Processor::finished() { return finished_; }