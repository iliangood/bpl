#if !defined PROCESSOR_H
#define PROCESSOR_H

#include <iostream>
#include <vector>
#include <string>
#include <variant>

#include "variables/stack.h"
#include "variables/type.h"


enum class OpCode
{
	end_ = 0,

	call_,
	ret_,
	
	init_,
	get_, // get value from StackLink(StackLink from PreStackLink)
	set_, // set value by StackLink
	valfromstlink_, // value from StackLink
	valfromarg_, // get value from argument

	
	if_,
	while_,
	
	runInstsVec_,

	add_,
	sub_,
	mul_,
	div_,
	mod_,
	and_,
	or_,
	not_,
	shl_,
	shr_,

	stackRealloc_,

	printCh_,
	readCh_,

	ls_, // lesser
	leq_, // lesser or equals
	bg_, // bigger
	beq_, // bigger or equals
	equ_, // equals
	neq_ // not equals
};

enum class Condition
{
	eq_,
	neq_,
	lt_,
	lte_,
	gt_,
	gte_
};


enum class BaseTypeId
{
	int64_ = 0,
	bool_,
	char_,
	double_,
	void_,
	countOfBaseTypes
};

class BaseTypeVector : public std::vector<BaseType>
{
public:
	using std::vector<BaseType>::vector;
	using std::vector<BaseType>::operator[];
	using std::vector<BaseType>::at;
	using std::vector<BaseType>::size;
	using std::vector<BaseType>::resize;
	using std::vector<BaseType>::begin;
	using std::vector<BaseType>::end;
	using std::vector<BaseType>::data;
	BaseType& operator[](BaseTypeId id) { return std::vector<BaseType>::operator[](static_cast<size_t>(id)); }
	const BaseType& operator[](BaseTypeId id) const { return std::vector<BaseType>::operator[](static_cast<size_t>(id)); }
	void resize(BaseTypeId count) { std::vector<BaseType>::resize(static_cast<size_t>(count)); }
};
class Processor;

class PreStackIndex
{
	size_t index_;
	bool global_;
public:
	PreStackIndex(size_t index, bool isGlobal = false) : index_(index), global_(isGlobal) {}
	size_t index() { return index_; }
	bool isGlobal() { return global_; }
	bool global() { return global_; }
};

class StackIndex
{
	size_t index_;
	Processor* processor_;
public:
	StackIndex(size_t index, Processor* processor, bool isGlobal = false);
	StackIndex(Element element, Processor* processor);
	StackIndex(PreStackIndex preStackIndex, Processor* processor) : StackIndex(preStackIndex.index(), processor, preStackIndex.isGlobal()) {}

	size_t index() const { return index_; }

	StackIndex(const StackIndex& other) : index_(other.index_), processor_(other.processor_) {}
	StackIndex(StackIndex&& other) : index_(other.index_), processor_(other.processor_)
	{
		other.index_ = 0;
		other.processor_ = nullptr;
	}

	StackIndex& operator=(const StackIndex& other)
	{
		index_ = other.index_;
		return *this;
	}
	
	StackIndex& operator=(StackIndex&& other)
	{
		index_ = other.index_;
		other.index_ = 0;
		return *this;
	}

	bool isValid() const;
};

typedef std::variant<size_t, uint8_t*> Link;

class Instruction;

class Function 
{
	FunctionType type_;
	std::vector<Instruction> body_;
public:
	Function() : type_(), body_() {}
	Function(FunctionType type, const std::vector<Instruction>& body) : type_(type), body_(body){}
	Function(const Function& other) : type_(other.type_), body_(other.body_) {}
	Function(Function&& other) : type_(std::move(other.type_)), body_(std::move(other.body_)) {}

	Function& operator=(const Function& other);

	Function& operator=(Function&& other);

	bool isValid() const;

	FunctionType& type() { return type_; }
	const FunctionType& type() const { return type_; }
	std::vector<Instruction>& body() { return body_; }
	const std::vector<Instruction>& body() const { return body_; }
};

typedef std::variant<int64_t, char, Function> Value;

typedef std::variant<Condition, TypeVariant, std::vector<Instruction>, PreStackIndex, Value> Argument;



class Instruction
{
	OpCode opCode_;
	std::vector<Argument> arguments_;
public:
	Instruction(OpCode opCode, std::vector<Argument> arguments_ = {}) : opCode_(opCode), arguments_(arguments_) {}
	Instruction(const Instruction& other) : opCode_(other.opCode_), arguments_(other.arguments_) {}
	Instruction(Instruction&& other) : opCode_(other.opCode_), arguments_(std::move(other.arguments_)) {}
	Instruction& operator=(const Instruction& other)
	{
		opCode_ = other.opCode_;
		arguments_ = other.arguments_;
		return *this;
	}
	Instruction& operator=(Instruction&& other)
	{
		opCode_ = other.opCode_;
		arguments_ = std::move(other.arguments_);
		return *this;
	}
	OpCode opCode() const { return opCode_; }
	const std::vector<Argument>& arguments() const { return arguments_; }
	std::vector<Argument>& arguments() { return arguments_; }
};



class Processor
{
	friend class StackIndex;
	friend class Function;

	std::vector<Instruction> programm_;
	BaseTypeVector baseTypes_;
	Stack stack_;
	std::vector<size_t> functionStackStartPositions_;
	Stack FunctionReturnValues_;
	bool finished_;
	bool returningFromFunction_;
	std::vector<uint8_t> returningValue_;

	void functionEntry();
	void functionExit();

	std::optional<int64_t> mathOper(int64_t(*operFunc)(int64_t a, int64_t b));
	std::optional<int64_t> logicOper(bool(*operFunc)(bool a, bool b));
	std::optional<int64_t> logicOper(bool(*operFunc)(bool a));

	std::optional<int64_t> compareOper(bool(*operFunc)(int64_t a, int64_t b));


	std::optional<int64_t> end_(Instruction& instruction);

	std::optional<int64_t> call_(Instruction& instruction);
	std::optional<int64_t> ret_(Instruction& instruction);

	std::optional<int64_t> scopeRet_(Instruction& instruction);
	std::optional<int64_t> init_(Instruction& instruction);
	std::optional<int64_t> get_(Instruction& instruction);
	std::optional<int64_t> set_(Instruction& instruction);
	std::optional<int64_t> valfromstlink_(Instruction& instruction);
	std::optional<int64_t> valfromarg_(Instruction& instruction);

	bool checkCondition(std::vector<Instruction>& condition);

	std::optional<int64_t> if_(Instruction& instruction);
	std::optional<int64_t> while_(Instruction& instruction);

	std::optional<int64_t> runInstsVec_(Instruction& instruction);

	std::optional<int64_t> add_(Instruction& instruction);
	std::optional<int64_t> sub_(Instruction& instruction);
	std::optional<int64_t> mul_(Instruction& instruction);
	std::optional<int64_t> div_(Instruction& instruction);
	std::optional<int64_t> mod_(Instruction& instruction);
	std::optional<int64_t> and_(Instruction& instruction);
	std::optional<int64_t> or_(Instruction& instruction);
	std::optional<int64_t> not_(Instruction& instruction);
	std::optional<int64_t> shl_(Instruction& instruction);
	std::optional<int64_t> shr_(Instruction& instruction);

	std::optional<int64_t> stackRealloc_(Instruction& instruction);

	std::optional<int64_t> printCh_(Instruction& instruction);
	std::optional<int64_t> readCh_(Instruction& instruction);

	std::optional<int64_t> ls_(Instruction& instruction);
	std::optional<int64_t> leq_(Instruction& instruction);
	std::optional<int64_t> bg_(Instruction& instruction);
	std::optional<int64_t> beq_(Instruction& instruction);
	std::optional<int64_t> equ_(Instruction& instruction);
	std::optional<int64_t> neq_(Instruction& instruction);
	

	std::optional<int64_t> execute(Instruction& instruction);

	bool returningFromFunction() const { return returningFromFunction_; }
	public:
	Processor(const std::vector<Instruction>& program, size_t stackSize = 1 << 20);
	std::optional<int64_t> run();
	void notifyStackReallocation(uint8_t* new_data);

	bool finished();
};

#endif