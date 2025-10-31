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



Function::Function(StackIndex index, Processor* processor) : processor_(processor) // ! Переделать
{
	if(processor_ == nullptr)
		throw std::invalid_argument("Function::Function(StackIndex, Processor*) null Processor pointer");
	if(!index.isValid())
		throw std::invalid_argument("Function::Function(StackIndex, Processor*) invalid StackIndex");
	auto optFuncElement = processor_->stack_.element(index.index());
	if(!optFuncElement.has_value())
		throw std::invalid_argument("Function::Function(StackIndex, Processor*) invalid stack element");
	Element funcElement = *optFuncElement;
	if(!funcElement.type().isFunctionType())
		throw std::invalid_argument("Function::Function(StackIndex, Processor*) index does not point to a FunctionType");
	type_ = std::get<FunctionType>(funcElement.type());
	auto optPtr = processor_->stack_.at(funcElement.pos());
	if(!optPtr.has_value())
		throw std::invalid_argument("Function::Function(StackIndex, Processor*) function body pointer not found on stack");
	auto ptr = reinterpret_cast<std::vector<Instruction>*>(*optPtr);
	body_ = *ptr;
}



Processor::Processor(const std::vector<Instruction>& program, size_t stackSize) : programm_(program),
stack_(this, stackSize), FunctionReturnValues_(this, 1024), finished_(false), returningFromFunction_(false)
{
	baseTypes_.resize(BaseTypeId::countOfBaseTypes);
	baseTypes_[BaseTypeId::int64_] = BaseType("int64", sizeof(int64_t));
	baseTypes_[BaseTypeId::uint64_] = BaseType("uint64", sizeof(uint64_t));
	baseTypes_[BaseTypeId::char_] = BaseType("char", sizeof(char));
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

std::optional<int64_t> Processor::end_(Instruction& instruction) // ! Переделать
{
	finished_ = true;
	if(instruction.arguments().size() == 0)
		return 0;
	if(instruction.arguments().size() == 1)
	{
	}
	throw std::runtime_error("Invalid end instruction");
}

std::optional<int64_t> Processor::call_(Instruction&)
{
	std::optional<Element> lastElemOpt = stack_.wholeElementFromEnd(0);
	if(!lastElemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::call_(Instruction&) called on incorrect stack stack_.wholeElementFromEnd(0) hasn't value");
	Element lastElem = lastElemOpt.value();
	if(!lastElem.type().isFunctionType())
		throw std::runtime_error("std::optional<int64_t> Processor::call_(Instruction&) called on non-function last stack element");
	FunctionType func = lastElem.type().get<FunctionType>(); 
	const std::vector<TypeVariant>& args = func.argumentsTypes();
	if(getValidationLevel() >= ValidationLevel::light)
	{
		for(size_t i = 0; i < args.size(); ++i)
		{
			std::optional<Element> we = stack_.wholeElementFromEnd(i+1);
			if(!we.has_value())
			{
				throw std::runtime_error("std::optional<int64_t> Processor::call_(Instruction&) function called on invalid arguments");
			}
			if(args[i] != we.value().type())
			{
				throw std::runtime_error("std::optional<int64_t> Processor::call_(Instruction&) function called on invalid arguments");
			}
		}
	}
	std::optional<uint8_t*> elemPtr = stack_.atWhole(lastElem.index());
	if(!elemPtr.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::call_(Instruction&) can't get element pointer");
	std::vector<Instruction>& body = *reinterpret_cast<std::vector<Instruction>*>(elemPtr.value());
	for(Instruction& inst : body)
	{
		execute(inst);
		if(returningFromFunction_)
		{
			returningFromFunction_ = false;
			functionExit();
			stack_.push(func.returnType());
			if(returningValue_.size() != func.returnType().size())
				throw std::runtime_error("std::optional<int64_t> Processor::call_(Instruction&) invalid return value");
			memcpy(stack_.atWholeFromEnd(0).value(), returningValue_.data(), returningValue_.size());
			break;
		}
	}
	return 0;
}

std::optional<int64_t> Processor::ret_(Instruction&)
{
	std::optional<Element> lastElemOpt = stack_.wholeElementFromEnd(0);
	if(!lastElemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::ret_(Instruction&) called on on incorrect stack");
	Element lastElem = lastElemOpt.value();
	returningValue_.resize(lastElem.size());
	memcpy(returningValue_.data(), stack_.atWholeFromEnd(0).value(), lastElem.size());
	return 0;
}

std::optional<int64_t> Processor::init_(Instruction& instruction)
{
	std::vector<Argument>& args = instruction.arguments();
	if(args.size() != 1)
		throw std::runtime_error("std::optional<int64_t> Processor::init_(Instruction&) init_ called with invalid arguments count");
	if(!std::holds_alternative<TypeVariant>(args[0]))
		throw std::runtime_error("std::optional<int64_t> Processor::init_(Instruction&) init_ called with invalid argument");
	stack_.push(ElementInfo(std::get<TypeVariant>(args[0])));
	return 0;
}

std::optional<int64_t> Processor::get_(Instruction& instruction)
{
	std::vector<Argument>& args = instruction.arguments();
	if(args.size() != 1)
		throw std::runtime_error("std::optional<int64_t> Processor::get_(Instruction&) called with invalid argument count");
	if(!std::holds_alternative<PreStackIndex>(args[0]))
		throw std::runtime_error("std::optional<int64_t> Processor::get_(Instruction&) called with invalid argument");
	StackIndex stackIndex(std::get<PreStackIndex>(args[0]), this);
	std::optional<Element> elemOpt = stack_.element(stackIndex.index());
	if(!elemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::get_(Instruction&) can't get element");
	Element elem = elemOpt.value();
	Link elemPos = elem.index();
	uint8_t* elemPosD = stack_.push(ElementInfo(LinkType()));
	memcpy(elemPosD, &elem, sizeof(Link));
	return 0;
}

std::optional<int64_t> Processor::set_(Instruction&)
{
	std::optional<Element> valueElemOpt = stack_.wholeElementFromEnd(0);
	std::optional<Element> linkElemOpt = stack_.wholeElementFromEnd(1);
	if(!valueElemOpt.has_value() || !linkElemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::set_(Instruction&) called on invalid arguments in stack");
	Element valueElem = valueElemOpt.value();
	Element linkElem = linkElemOpt.value();
	if(!linkElem.type().isLinkType())
		throw std::runtime_error("std::optional<int64_t> Processor::set_(Instruction&) called on invalid link argument");
	Link* linkElemPos = reinterpret_cast<Link*>(stack_.at(linkElem));
	size_t linkDataSize;
	uint8_t* linkDataPtr;
	if(std::holds_alternative<uint8_t*>(*linkElemPos))
	{
		linkDataPtr = std::get<uint8_t*>(*linkElemPos);
		std::optional<TypeVariant> targetTypeOpt = linkElem.type().get<LinkType>().pointsTo();
		if(!targetTypeOpt.has_value())
			throw std::runtime_error("std::optional<int64_t> Processor::set_(Instruction&) called on invalid link pointsTo");
		TypeVariant targetType = targetTypeOpt.value();
		linkDataSize = targetType.size();
		if(getValidationLevel() >= ValidationLevel::light)
		{
			if(valueElem.type() != targetType)
				throw std::runtime_error("std::optional<int64_t> Processor::set_(Instruction&) incopatible link");

		}
	}
	else // size_t
	{
		std::optional<Element> linkedElementOpt = stack_.element(std::get<size_t>(*linkElemPos));
		if(!linkedElementOpt.has_value())
			throw std::runtime_error("std::optional<int64_t> Processor::set_(Instruction&) invalid link");
		Element linkedElement = linkedElementOpt.value();
		linkDataPtr = stack_.at(linkedElement);
		linkDataSize = linkedElement.type().size();
		if(getValidationLevel() >= ValidationLevel::light)
		{
			if(valueElem.type() != linkedElement.type())
				throw std::runtime_error("std::optional<int64_t> Processor::set_(Instruction&) incopatible link");
		}
	}
	memcpy(linkDataPtr, stack_.at(valueElem), linkDataSize);
	return 0;
}

std::optional<int64_t> Processor::valfromstlink_(Instruction&)
{
	std::optional<Element> elemOpt = stack_.wholeElementFromEnd(0);
	if(!elemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::valfromstlink_(Instruction&) called on empty stack");
	Element elem = elemOpt.value();
	if(!elem.type().isLinkType())
		throw std::runtime_error("std::optional<int64_t> Processor::valfromstlink_(Instruction&) last element should be link");
	Link link = *reinterpret_cast<Link*>(stack_.at(elem));
	size_t linkDataSize;
	uint8_t* linkDataPtr;
	TypeVariant linkType;
	if(std::holds_alternative<uint8_t*>(link))
	{
		linkDataPtr = std::get<uint8_t*>(link);
		std::optional<TypeVariant> linkTypeOpt = std::get<LinkType>(elem.type()).pointsTo();
		if(!linkTypeOpt.has_value())
			throw std::runtime_error("std::optional<int64_t> Processor::set_(Instruction&) called on invalid link pointsTo");
		linkType = linkTypeOpt.value();
		linkDataSize = linkType.size();
	}
	else // size_t
	{
		std::optional<Element> linkedElementOpt = stack_.element(std::get<size_t>(link));
		if(!linkedElementOpt.has_value())
			throw std::runtime_error("std::optional<int64_t> Processor::set_(Instruction&) invalid link");
		Element linkedElement = linkedElementOpt.value();
		linkType = linkedElement.type();
		linkDataPtr = stack_.at(linkedElement);
		linkDataSize = linkedElement.type().size();
	}
	stack_.pop();
	uint8_t* data = stack_.push(ElementInfo(linkType));
	memcpy(data, linkDataPtr, linkDataSize);
	return 0;
}


std::optional<int64_t> Processor::execute(Instruction& instruction)
{
	if(finished())
	{
		return std::nullopt;
	}
	switch (instruction.opCode())
	{
	case OpCode::end_:
		return end_(instruction);
		break;
	case OpCode::call_:
		return call_(instruction);
		break;
	case OpCode::ret_:
		return ret_(instruction);
		break;
	case OpCode::init_:
		return init_(instruction);
		break;
	case OpCode::get_:
		return get_(instruction);
		break;
	case OpCode::set_: 
		return set_(instruction);
		break;
	case OpCode::valfromstlink_: 
		return valfromstlink_(instruction);
		break;
	case OpCode::if_:
		return if_(instruction);
		break;
	case OpCode::while_:
		return while_(instruction);
		break;
	case OpCode::add_:
		return add_(instruction);
		break;
	case OpCode::sub_:
		return sub_(instruction);
		break;
	case OpCode::mul_:
		return mul_(instruction);
		break;
	case OpCode::div_:
		return div_(instruction);
		break;
	case OpCode::mod_:
		return mod_(instruction);
		break;
	case OpCode::and_:
		return and_(instruction);
		break;
	case OpCode::or_:
		return or_(instruction);
		break;
	case OpCode::not_:
		return not_(instruction);
		break;
	case OpCode::shl_:
		return shl_(instruction);
		break;
	case OpCode::shr_:
		return shr_(instruction);;
		break;
	case OpCode::stackRealloc_:
		return stackRealloc_(instruction);
		break;
	case OpCode::print_:
		return print_(instruction);
		break;
	case OpCode::scan_:
		return scan_(instruction);
		break;
	case OpCode::cmp_:
		return cmp_(instruction);
		break;
	default:
		break;
	}
	return std::nullopt;
}	

void Processor::notifyStackReallocation(uint8_t* /*new_data*/) // Currently does nothing
{
	//execute(Instruction(OpCode::stackRealloc_, {}));
}

bool Processor::finished() { return finished_; }