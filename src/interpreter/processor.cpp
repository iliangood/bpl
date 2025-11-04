#include "interpreter/processor.h"
#include "utils.h"
#include <sys/select.h>
#include <unistd.h>

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



Processor::Processor(const std::vector<Instruction>& program, size_t stackSize) : program_(program),
stack_(this, stackSize), FunctionReturnValues_(this, 1024), finished_(false), returningFromFunction_(false)
{
	baseTypes_.resize(BaseTypeId::countOfBaseTypes);
	baseTypes_[BaseTypeId::int64_] = BaseType("int64", sizeof(int64_t));
	baseTypes_[BaseTypeId::bool_] = BaseType("bool", sizeof(bool));
	baseTypes_[BaseTypeId::char_] = BaseType("char", sizeof(char));
	baseTypes_[BaseTypeId::double_] = BaseType("double", sizeof(double));
	baseTypes_[BaseTypeId::void_] = BaseType("void", 0);
	noBlockingInput_ = false;
}

Processor::Processor(size_t stackSize) : 
stack_(this, stackSize), FunctionReturnValues_(this, 1024), finished_(false), returningFromFunction_(false)
{
	//std::cout << "start" << std::endl;
	baseTypes_.resize(BaseTypeId::countOfBaseTypes);
	baseTypes_[BaseTypeId::int64_] = BaseType("int64", sizeof(int64_t));
	baseTypes_[BaseTypeId::bool_] = BaseType("bool", sizeof(bool));
	baseTypes_[BaseTypeId::char_] = BaseType("char", sizeof(char));
	baseTypes_[BaseTypeId::double_] = BaseType("double", sizeof(double));
	//std::cout << "checking" << std::endl;
	baseTypes_[BaseTypeId::void_] = BaseType("void", 0);
	//std::cout << "end" << std::endl;
	noBlockingInput_ = false;
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
	if(finished_)
		return std::nullopt;
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
	uint8_t* elemPtr = stack_.at(lastElem);
	std::vector<Instruction>& body = **reinterpret_cast<std::vector<Instruction>**>(elemPtr);
	for(Instruction& inst : body)
	{
		execute(inst);
		if(finished_)
		{
			functionExit();
			return std::nullopt;
		}
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
	if(finished_)
		return std::nullopt;
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
	if(finished_)
		return std::nullopt;
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
	if(finished_)
		return std::nullopt;
	//std::cout << "getting" << std::endl;
	std::vector<Argument>& args = instruction.arguments();
	if(args.size() != 1)
		throw std::runtime_error("std::optional<int64_t> Processor::get_(Instruction&) called with invalid argument count");
	if(!std::holds_alternative<PreStackIndex>(args[0]))
		throw std::runtime_error("std::optional<int64_t> Processor::get_(Instruction&) called with invalid argument");
	//std::cout << "ready getting01" << std::endl;
	StackIndex stackIndex(std::get<PreStackIndex>(args[0]), this);
	//std::cout << "ready getting0" << std::endl;
	std::optional<Element> elemOpt = stack_.element(stackIndex.index());
	if(!elemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::get_(Instruction&) can't get element");
	//std::cout << "ready getting" << std::endl;
	Element& elem = elemOpt.value();
	Link elemPos = elem.index();
	LinkType linkType = LinkType();
	TypeVariant linkVariant(linkType);
	ElementInfo elemInfo(linkVariant);
	uint8_t* elemPosD = stack_.push(ElementInfo(TypeVariant(LinkType())));
	*reinterpret_cast<Link*>(elemPosD) = elemPos;
	return 0;
}

std::optional<int64_t> Processor::set_(Instruction&)
{
	if(finished_)
		return std::nullopt;
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
	stack_.pop();
	stack_.pop();
	return 0;
}

std::optional<int64_t> Processor::valfromstlink_(Instruction&)
{
	if(finished_)
		return std::nullopt;
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

std::optional<int64_t> Processor::getSublink_(Instruction&)
{
	if(finished_)
		return std::nullopt;
	std::optional<Element> elemOpt = stack_.wholeElementFromEnd(1);
	std::optional<Element> elemDubIndexOpt = stack_.wholeElementFromEnd(0);
	if(!elemOpt.has_value() || !elemDubIndexOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::getSublink_(Instruction&) called on incorrect stack");
	Element elem = elemOpt.value();
	Element elemDubIndex = elemDubIndexOpt.value();
	if(!elem.type().isLinkType() || elemDubIndex.type().isBaseType())
		throw std::runtime_error("std::optional<int64_t> Processor::getSublink_(Instruction&) incorrect elemnts types");
	if(elemDubIndex.type() != &baseTypes_[BaseTypeId::int64_])
		throw std::runtime_error("std::optional<int64_t> Processor::getSublink_(Instruction&) last element should be int64");
	Link link = *reinterpret_cast<Link*>(stack_.at(elem));
	size_t subIndex = *reinterpret_cast<int64_t*>(stack_.at(elemDubIndex));

	if(subIndex == 0)
	{
		stack_.pop();
		return 0;
	}
	TypeVariant linkType;
	Link subLink;
	if(std::holds_alternative<uint8_t*>(link))
	{
		std::optional<TypeVariant> linkTypeOpt = elem.type().get<LinkType>().pointsTo();
		if(!linkTypeOpt.has_value())
			throw std::runtime_error("std::optional<int64_t> Processor::getSublink_(Instruction&) incorrect elemnt info in stack");
		linkType = linkTypeOpt.value();
		if(linkType.isArrayType())
		{
			TypeVariant arrType = linkType.get<ArrayType>().elementType();
			size_t elemSize = arrType.size();
			size_t elemCount = linkType.get<ArrayType>().count();
			if(subIndex > elemCount)
				throw std::out_of_range("std::optional<int64_t> Processor::getSublink_(Instruction&) subIndex > elemCount in array");
			size_t offset = (subIndex - 1) * elemSize;
			subLink = std::get<uint8_t*>(link) + offset;
			stack_.pop();
			stack_.pop();
			uint8_t* dataPtr = stack_.push(TypeVariant(LinkType(linkType)));
			*reinterpret_cast<Link*>(dataPtr) = subLink;
			return 0;
		}
		if(linkType.isStructType())
		{
			size_t offset = linkType.get<const StructType*>()->offsetBySize(subIndex);
			TypeVariant elemType = linkType.get<const StructType*>()->type(subIndex);
			subLink = std::get<uint8_t*>(link) + offset;
			stack_.pop();
			stack_.pop();
			uint8_t* dataPtr = stack_.push(TypeVariant(LinkType(linkType)));
			*reinterpret_cast<Link*>(dataPtr) = subLink;
			return 0;
		}
		throw std::runtime_error("std::optional<int64_t> Processor::getSublink_(Instruction&) incorrect link's pointsTo");
	}
	else if(std::holds_alternative<size_t>(link))
	{
		std::optional<Element> linkedElementOpt = stack_.element(std::get<size_t>(link));
		if(!linkedElementOpt.has_value())
			throw std::runtime_error("std::optional<int64_t> Processor::getSublink_(Instruction&) incorrect link");
		Element linkedElement = linkedElementOpt.value();
		std::optional<Element> subElementOpt = linkedElement.atSubElements(subIndex);
		if(!subElementOpt.has_value())
			throw std::runtime_error("std::optional<int64_t> Processor::getSublink_(Instruction&) incorrect sub-index");
		Element subElement = subElementOpt.value();
		subLink = subElement.index();
		stack_.pop();
		stack_.pop();
		uint8_t* dataPtr = stack_.push(TypeVariant(LinkType()));
		*reinterpret_cast<Link*>(dataPtr) = subLink;
	}
	else
		throw std::runtime_error("std::optional<int64_t> Processor::getSublink_(Instruction&) unknown link type");
	return 0;
}

std::optional<int64_t> Processor::valfromarg_(Instruction& instruction)
{
	if(finished_)
		return std::nullopt;
	std::vector<Argument>& args = instruction.arguments();
	if(args.size() != 1)
		throw std::runtime_error("std::optional<int64_t> valfromarg_(Instruction&) incorrect arguments count");
	//std::cout << args[0].index() << std::endl;
	if(!std::holds_alternative<Value>(args[0]))
		throw std::runtime_error("std::optional<int64_t> valfromarg_(Instruction&) incorrect argument type");
	Value& val = std::get<Value>(args[0]);
	if(std::holds_alternative<int64_t>(val))
	{
		int64_t value = std::get<int64_t>(val);
		uint8_t* addr = stack_.push(ElementInfo(&baseTypes_[BaseTypeId::int64_]));
		*reinterpret_cast<int64_t*>(addr) = value;
		return 0;
	}
	if(std::holds_alternative<char>(val))
	{
		char value = std::get<char>(val);
		uint8_t* addr = stack_.push(ElementInfo(&baseTypes_[BaseTypeId::char_]));
		//std::cout << "put " << static_cast<int>(value) << "in stack" << std::endl;
		*reinterpret_cast<char*>(addr) = value;
		return 0;
	}
	if(std::holds_alternative<Function>(val))
	{
		Function& func = std::get<Function>(val);
		FunctionType& funcType = func.type();
		TypeVariant& returnType = funcType.returnType();
		std::vector<TypeVariant>& argumentsTypes = funcType.argumentsTypes();
		uint8_t* addr = stack_.push(ElementInfo(FunctionType(argumentsTypes, returnType)));
		std::vector<Instruction>* bodyAddr = &func.body();
		*reinterpret_cast<std::vector<Instruction>**>(addr) = bodyAddr;
		return 0;
	}
	return 0;

}


bool Processor::checkCondition(std::vector<Instruction>& condition)
{
	stack_.newLevel();
	for(Instruction& inst : condition)
	{
		if(finished_)
		{
			stack_.popLevel();
			return 0;
		}
		if(returningFromFunction_)
		{
			stack_.popLevel();
			return 0;
		}
		execute(inst);
	}
	std::optional<Element> condResElemOpt = stack_.wholeElementFromEnd(0);
	if(!condResElemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::checkCondition(Instruction&) incorrect condition: no return value");
	Element condResElem = condResElemOpt.value();
	if(!condResElem.type().isBaseType())
		throw std::runtime_error("std::optional<int64_t> Processor::checkCondition(Instruction&) incorrect condition: incorrect return value: should be BaseType bool");

	if(condResElem.type().get<const BaseType*>() != &baseTypes_[BaseTypeId::bool_])
		throw std::runtime_error("std::optional<int64_t> Processor::checkCondition(Instruction&) incorrect condition: incorrect return value: should be BaseType bool");
	bool res = *reinterpret_cast<bool*>(stack_.at(condResElem));
	stack_.popLevel();
	return res;
}

std::optional<int64_t> Processor::if_(Instruction& instruction)
{
	std::vector<Argument>& args = instruction.arguments();
	if(!(args.size() == 2 || args.size() == 3))
		throw std::runtime_error("std::optional<int64_t> Processor::if_(Instruction&) with incorrect argumets count");
	if(!std::holds_alternative<std::vector<Instruction>>(args[0]) || !std::holds_alternative<std::vector<Instruction>>(args[1]))
		throw std::runtime_error("std::optional<int64_t> Processor::if_(Instruction&) with incorrect argumets types");
	if(args.size() == 3)
	{
		if(!std::holds_alternative<std::vector<Instruction>>(args[2]))
			throw std::runtime_error("std::optional<int64_t> Processor::if_(Instruction&) with incorrect argumets types");
	}
	std::vector<Instruction>& condition = std::get<std::vector<Instruction>>(args[0]);
	bool condRes = checkCondition(condition);
	if(returningFromFunction_)
		return 0;
	if(finished_)
		return std::nullopt;
	if(condRes)
	{
		std::vector<Instruction>& insts = std::get<std::vector<Instruction>>(args[1]);
		stack_.newLevel();
		for(Instruction& inst : insts)
		{
			if(returningFromFunction_)
			{
				stack_.popLevel();
				return 0;
			}
			if(finished_)
			{
				stack_.popLevel();
				return std::nullopt;
			}
			execute(inst);
		}
		stack_.popLevel();
	}
	if(!condRes && args.size() == 3)
	{
		std::vector<Instruction>& insts = std::get<std::vector<Instruction>>(args[2]);
		stack_.newLevel();
		for(Instruction& inst : insts)
		{
			if(returningFromFunction_)
			{
				stack_.popLevel();
				return 0;
			}
			if(finished_)
			{
				stack_.popLevel();
				return std::nullopt;
			}
			execute(inst);
		}
		stack_.popLevel();
	}
	return 0;
}

std::optional<int64_t> Processor::while_(Instruction& instruction)
{
	std::vector<Argument>& args = instruction.arguments();
	if(args.size() != 2)
		throw std::runtime_error("std::optional<int64_t> Processor::while_(Instruction&) incorrect argumets count");
	if(!std::holds_alternative<std::vector<Instruction>>(args[0]) || !std::holds_alternative<std::vector<Instruction>>(args[1]))
		throw std::runtime_error("std::optional<int64_t> Processor::while_(Instruction&) incorrect argumets types");
	std::vector<Instruction>& condition = std::get<std::vector<Instruction>>(args[0]);
	bool condRes = checkCondition(condition);
	if(returningFromFunction_)
		return 0;
	if(finished_)
		return std::nullopt;
	std::vector<Instruction>& body = std::get<std::vector<Instruction>>(args[1]);
	while(condRes)
	{
		stack_.newLevel();
		for(Instruction& inst : body)
		{
			if(returningFromFunction_)
			{
				stack_.popLevel();
				return 0;
			}
			if(finished_)
			{
				stack_.popLevel();
				return std::nullopt;
			}
			execute(inst);
		}
		stack_.popLevel();
		condRes = checkCondition(condition);
		if(returningFromFunction_)
			return 0;
		if(finished_)
			return std::nullopt;
	}
	return 0;
}

std::optional<int64_t> Processor::runInstsVec_(Instruction& instruction)
{
	if(finished_)
		return std::nullopt;
	std::vector<Argument>& args = instruction.arguments();
	if(args.size() != 1)
		throw std::runtime_error("std::optional<int64_t> Processor::runInstsVec_(Instruction&) incorrect arguments count");
	if(!std::holds_alternative<std::vector<Instruction>>(args[0]))
		throw std::runtime_error("std::optional<int64_t> Processor::runInstsVec_(Instruction&) incorrect argument type");
	std::vector<Instruction>& body = std::get<std::vector<Instruction>>(args[0]);
	stack_.newLevel();
	for(Instruction& inst : body)
	{
		if(returningFromFunction_)
		{
			stack_.popLevel();
			return 0;
		}
		if(finished_)
		{
			stack_.popLevel();
			return std::nullopt;
		}
		execute(inst);
	}
	stack_.popLevel();
	return 0;
}

std::optional<int64_t> Processor::mathOper(int64_t(*operFunc)(int64_t a, int64_t b))
{
	std::optional<Element> operAElemOpt = stack_.wholeElementFromEnd(1);
	std::optional<Element> operBElemOpt = stack_.wholeElementFromEnd(0);
	if(!operAElemOpt.has_value() || !operBElemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::mathOper(int64_t(*operFunc)(int64_t a, int64_t b)) invalid stack: can't get value");
	Element operAElem = operAElemOpt.value();
	Element operBElem = operBElemOpt.value();
	if(!operAElem.type().isBaseType() || !operBElem.type().isBaseType())
		throw std::runtime_error("std::optional<int64_t> Processor::mathOper(int64_t(*operFunc)(int64_t a, int64_t b)) invalid argumets types");
	const BaseType* operAType = operAElem.type().get<const BaseType*>();
	const BaseType* operBType = operBElem.type().get<const BaseType*>();
	if(operAType == &baseTypes_[BaseTypeId::int64_] && operBType == &baseTypes_[BaseTypeId::int64_])
	{
		int64_t operA = *reinterpret_cast<int64_t*>( stack_.at(operAElem));
		int64_t operB = *reinterpret_cast<int64_t*>( stack_.at(operBElem));
		stack_.pop();
		stack_.pop();
		int64_t res = operFunc(operA, operB);
		uint8_t* resAddr = stack_.push(ElementInfo(TypeVariant(&baseTypes_[BaseTypeId::int64_])));
		*reinterpret_cast<int64_t*>(resAddr) = res;
		return 0;
	}
	if(operAType == &baseTypes_[BaseTypeId::char_] && operBType == &baseTypes_[BaseTypeId::char_])
	{
		char operA = *reinterpret_cast<char*>( stack_.at(operAElem));
		char operB = *reinterpret_cast<char*>( stack_.at(operBElem));
		stack_.pop();
		stack_.pop();
		char res = operFunc(operA, operB);
		uint8_t* resAddr = stack_.push(ElementInfo(TypeVariant(&baseTypes_[BaseTypeId::char_])));
		*reinterpret_cast<char*>(resAddr) = res;
		return 0;
	}
	throw std::runtime_error("std::optional<int64_t> Processor::mathOper(int64_t(*operFunc)(int64_t a, int64_t b)) incorrect argumets types");
	return 0;
}

std::optional<int64_t> Processor::logicOper(bool(*operFunc)(bool a, bool b))
{
	std::optional<Element> operAElemOpt = stack_.wholeElementFromEnd(1);
	std::optional<Element> operBElemOpt = stack_.wholeElementFromEnd(0);
	if(!operAElemOpt.has_value() || !operBElemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::logicOper(bool(*operFunc)(int64_t a, int64_t b)) invalid stack: can't get value");
	Element operAElem = operAElemOpt.value();
	Element operBElem = operBElemOpt.value();
	if(!operAElem.type().isBaseType() || !operBElem.type().isBaseType())
		throw std::runtime_error("std::optional<int64_t> Processor::logicOper(bool(*operFunc)(int64_t a, int64_t b)) invalid argumets types");
	const BaseType* operAType = operAElem.type().get<const BaseType*>();
	const BaseType* operBType = operBElem.type().get<const BaseType*>();
	if(operAType == &baseTypes_[BaseTypeId::bool_] && operBType == &baseTypes_[BaseTypeId::bool_])
	{
		bool operA = *reinterpret_cast<bool*>( stack_.at(operAElem));
		bool operB = *reinterpret_cast<bool*>( stack_.at(operBElem));
		stack_.pop();
		stack_.pop();
		bool res = operFunc(operA, operB);
		uint8_t* resAddr = stack_.push(ElementInfo(TypeVariant(&baseTypes_[BaseTypeId::int64_])));
		*reinterpret_cast<bool*>(resAddr) = res;
		return 0;
	}
	throw std::runtime_error("std::optional<int64_t> Processor::logicOper(bool(*operFunc)(bool a, bool b)) incorrect argumets types");
	return 0;
}

std::optional<int64_t> Processor::logicOper(bool(*operFunc)(bool a))
{
	std::optional<Element> operElemOpt = stack_.wholeElementFromEnd(0);
	if(!operElemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::logicOper(bool(*operFunc)(bool a)) invalid stack: can't get value");
	Element operElem = operElemOpt.value();
	if(!operElem.type().isBaseType())
		throw std::runtime_error("std::optional<int64_t> Processor::logicOper(bool(*operFunc)(bool a)) invalid argumets types");
	const BaseType* operType = operElem.type().get<const BaseType*>();
	if(operType == &baseTypes_[BaseTypeId::bool_])
	{
		bool oper = *reinterpret_cast<bool*>( stack_.at(operElem));
		stack_.pop();
		bool res = operFunc(oper);
		uint8_t* resAddr = stack_.push(ElementInfo(TypeVariant(&baseTypes_[BaseTypeId::int64_])));
		*reinterpret_cast<bool*>(resAddr) = res;
		return 0;
	}
	throw std::runtime_error("std::optional<int64_t> Processor::logicOper(bool(*operFunc)(bool a)) incorrect argumets types");
	return 0;
}

std::optional<int64_t> Processor::compareOper(bool(*operFunc)(int64_t a, int64_t b))
{
	std::optional<Element> operAElemOpt = stack_.wholeElementFromEnd(1);
	std::optional<Element> operBElemOpt = stack_.wholeElementFromEnd(0);
	if(!operAElemOpt.has_value() || !operBElemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::compareOper(bool(*operFunc)(int64_t a, int64_t b)) invalid stack: can't get value");
	Element operAElem = operAElemOpt.value();
	Element operBElem = operBElemOpt.value();
	if(!operAElem.type().isBaseType() || !operBElem.type().isBaseType())
		throw std::runtime_error("std::optional<int64_t> Processor::compareOper(bool(*operFunc)(int64_t a, int64_t b)) invalid argumets types");
	const BaseType* operAType = operAElem.type().get<const BaseType*>();
	const BaseType* operBType = operBElem.type().get<const BaseType*>();
	if(operAType == &baseTypes_[BaseTypeId::int64_] && operBType == &baseTypes_[BaseTypeId::int64_])
	{
		int64_t operA = *reinterpret_cast<int64_t*>( stack_.at(operAElem));
		int64_t operB = *reinterpret_cast<int64_t*>( stack_.at(operBElem));
		stack_.pop();
		stack_.pop();
		bool res = operFunc(operA, operB);
		uint8_t* resAddr = stack_.push(ElementInfo(TypeVariant(&baseTypes_[BaseTypeId::bool_])));
		*reinterpret_cast<bool*>(resAddr) = res;
		return 0;
	}
	if(operAType == &baseTypes_[BaseTypeId::char_] && operBType == &baseTypes_[BaseTypeId::char_])
	{
		char operA = *reinterpret_cast<char*>( stack_.at(operAElem));
		char operB = *reinterpret_cast<char*>( stack_.at(operBElem));
		stack_.pop();
		stack_.pop();
		bool res = operFunc(operA, operB);
		uint8_t* resAddr = stack_.push(ElementInfo(TypeVariant(&baseTypes_[BaseTypeId::bool_])));
		*reinterpret_cast<char*>(resAddr) = res;
		return 0;
	}
	throw std::runtime_error("std::optional<int64_t> Processor::compareOper(bool(*operFunc)(int64_t a, int64_t b)) incorrect argumets types");
	return 0;
}


std::optional<int64_t> Processor::add_(Instruction&)
{
	return mathOper([](int64_t a, int64_t b){ return a + b; });
}

std::optional<int64_t> Processor::sub_(Instruction&)
{
	return mathOper([](int64_t a, int64_t b){ return a - b; });
}
std::optional<int64_t> Processor::mul_(Instruction&)
{
	return mathOper([](int64_t a, int64_t b){ return a * b; });
}
std::optional<int64_t> Processor::div_(Instruction&)
{
	return mathOper([](int64_t a, int64_t b){ return a / b; });
}
std::optional<int64_t> Processor::mod_(Instruction&)
{
	return mathOper([](int64_t a, int64_t b){ return a % b; });
}

std::optional<int64_t> Processor::shl_(Instruction&)
{
	return mathOper([](int64_t a, int64_t b){ return a << b; });
}
std::optional<int64_t> Processor::shr_(Instruction&)
{
	return mathOper([](int64_t a, int64_t b){ return a >> b; });
}

std::optional<int64_t> Processor::and_(Instruction&)
{
	return logicOper([](bool a, bool b){ return a && b; });
}
std::optional<int64_t> Processor::or_(Instruction&)
{
	return logicOper([](bool a, bool b){ return a && b; });
}
std::optional<int64_t> Processor::not_(Instruction&)
{
	return logicOper([](bool a){ return !a; });
}

std::optional<int64_t> Processor::ls_(Instruction&)
{
	return compareOper([](int64_t a, int64_t b){ return a < b; });
}
std::optional<int64_t> Processor::leq_(Instruction&)
{
	return compareOper([](int64_t a, int64_t b){ return a <= b; });
}
std::optional<int64_t> Processor::bg_(Instruction&)
{
	return compareOper([](int64_t a, int64_t b){ return a > b; });
}
std::optional<int64_t> Processor::beq_(Instruction&)
{
	return compareOper([](int64_t a, int64_t b){ return a >= b; });
}
std::optional<int64_t> Processor::equ_(Instruction&)
{
	return compareOper([](int64_t a, int64_t b){ return a == b; });
}
std::optional<int64_t> Processor::neq_(Instruction&)
{
	return compareOper([](int64_t a, int64_t b){ return a != b; });
}

bool has_input_nonblocking() 
{
	fd_set readfds;
	struct timeval timeout = {0, 0};

	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);

	int result = select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout);

	return result > 0; 
}

template<typename T>
std::optional<T> read(bool nonBlocking) 
{
	if(nonBlocking)
	{
		if(!has_input_nonblocking())
		{
			return std::nullopt;
		}
	}
	T val;
	std::cin >> val;
	return val;  
}

std::optional<int64_t> Processor::readCh_(Instruction&)
{

	std::optional<char> chOpt = read<char>(noBlockingInput_);
	char ch = chOpt.has_value() ? chOpt.value() : 0;
	uint8_t* ptr = stack_.push(ElementInfo(&baseTypes_[BaseTypeId::char_]));
	*reinterpret_cast<char*>(ptr) = ch;
	return 0;
}

std::optional<int64_t> Processor::checkBuf_(Instruction&)
{
	bool res = has_input_nonblocking();
	uint8_t* ptr = stack_.push(ElementInfo(&baseTypes_[BaseTypeId::bool_]));
	*reinterpret_cast<bool*>(ptr) = res;
	return 0;
}

std::optional<int64_t> Processor::setNoBlockingInput_(Instruction&)
{
	std::optional<Element> dataElemOpt = stack_.wholeElementFromEnd(0);
	if(!dataElemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::setNoBlockingInput_(Instruction&) invalid last stack element");
	Element dataElem = dataElemOpt.value();
	if(!dataElem.type().isBaseType())
		throw std::runtime_error("std::optional<int64_t> Processor::setNoBlockingInput_(Instruction&) incorrect last stack element");
	if(dataElem.type().get<const BaseType*>() != &baseTypes_[BaseTypeId::bool_])
		throw std::runtime_error("std::optional<int64_t> Processor::setNoBlockingInput_(Instruction&) incopatible last stack element");
	uint8_t* ptr = stack_.at(dataElem);
	noBlockingInput_ = *reinterpret_cast<bool*>(ptr);
	stack_.pop();
	return 0;
}

std::optional<int64_t> Processor::readNum_(Instruction&)
{

	std::optional<int64_t> chOpt = read<int64_t>(noBlockingInput_);
	int64_t num = chOpt.has_value() ? chOpt.value() : 0;
	uint8_t* ptr = stack_.push(ElementInfo(&baseTypes_[BaseTypeId::int64_]));
	*reinterpret_cast<int64_t*>(ptr) = num;
	return 0;
}

std::optional<int64_t> Processor::printCh_(Instruction&)
{
	std::optional<Element> dataElemOpt = stack_.wholeElementFromEnd(0);
	if(!dataElemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::printCh_(Instruction&) invalid last stack element");
	Element dataElem = dataElemOpt.value();
	if(!dataElem.type().isBaseType())
		throw std::runtime_error("std::optional<int64_t> Processor::printCh_(Instruction&) incorrect last stack element");
	if(dataElem.type().get<const BaseType*>() != &baseTypes_[BaseTypeId::char_])
		throw std::runtime_error("std::optional<int64_t> Processor::printCh_(Instruction&) incopatible last stack element");
	uint8_t* dataPtr = stack_.at(dataElem);
	char ch = *reinterpret_cast<char*>(dataPtr);
	stack_.pop();
	std::cout << ch;
	fflush(stdout);
	return 0;
}

std::optional<int64_t> Processor::printNum_(Instruction&)
{
	std::optional<Element> dataElemOpt = stack_.wholeElementFromEnd(0);
	if(!dataElemOpt.has_value())
		throw std::runtime_error("std::optional<int64_t> Processor::printNum_(Instruction&) invalid last stack element");
	Element dataElem = dataElemOpt.value();
	if(!dataElem.type().isBaseType())
		throw std::runtime_error("std::optional<int64_t> Processor::printNum_(Instruction&) incorrect last stack element");
	if(dataElem.type().get<const BaseType*>() != &baseTypes_[BaseTypeId::int64_])
		throw std::runtime_error("std::optional<int64_t> Processor::printNum_(Instruction&) incopatible last stack element");
	uint8_t* dataPtr = stack_.at(dataElem);
	int64_t num = *reinterpret_cast<int64_t*>(dataPtr);
	stack_.pop();
	std::cout << num;
	fflush(stdout);
	return 0;
}

std::optional<int64_t> Processor::peekCh_(Instruction&)
{
	char ch;
	if(noBlockingInput_ && !has_input_nonblocking())
		ch = 0;
	else
		std::cin >> ch;
	uint8_t* ptr = stack_.push(ElementInfo(&baseTypes_[BaseTypeId::char_]));
	*reinterpret_cast<char*>(ptr) = ch;
	return 0;
}

std::optional<int64_t> Processor::stackRealloc_(Instruction&) //заглушка
{
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
	case OpCode::valfromarg_: 
		return valfromarg_(instruction);
		break;
	case OpCode::getSublink_:
		return getSublink_(instruction);
		break;
	case OpCode::if_:
		return if_(instruction);
		break;
	case OpCode::while_:
		return while_(instruction);
		break;
	case OpCode::runInstsVec_:
		return runInstsVec_(instruction);
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
	case OpCode::setNoBlockingInput_:
		return setNoBlockingInput_(instruction);
		break;
	case OpCode::checkBuf_:
		return checkBuf_(instruction);
		break;
	case OpCode::printCh_:
		return printCh_(instruction);
		break;
	case OpCode::printNum_:
		return printNum_(instruction);
		break;
	case OpCode::readCh_:
		return readCh_(instruction);
		break;
	case OpCode::readNum_:
		return readNum_(instruction);
		break;
	case OpCode::ls_:
		return ls_(instruction);
		break;
	case OpCode::leq_:
		return leq_(instruction);
		break;
	case OpCode::bg_:
		return bg_(instruction);
		break;
	case OpCode::beq_:
		return beq_(instruction);
		break;
	case OpCode::equ_:
		return equ_(instruction);
		break;
	default:
		throw std::runtime_error("std::optional<int64_t> Processor::execute(Instruction&) unknown Opcode");
		break;
	}
	return std::nullopt;
}

std::optional<int64_t> Processor::run()
{
	//std::cout << "start execution" << std::endl;
	for(Instruction& inst : program_)
	{
		//std::cout << "executing" << std::endl;
		if(finished_)
			return std::nullopt;
		if(returningFromFunction_)
			return 0;
		execute(inst);
	}
	return 0;
}

void Processor::notifyStackReallocation(uint8_t* /*new_data*/) // Currently does nothing
{
	//execute(Instruction(OpCode::stackRealloc_, {}));
}

bool Processor::finished() { return finished_; }