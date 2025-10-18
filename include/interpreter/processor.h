#if !defined PROCESSOR_H
#define PROCESSOR_H

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

	scopeRet_,
	init_,
	mov_,

	if_,
	while_,

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

	print_,
	scan_,

	cmp_,
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


class Processor;

class StackIndex
{
	size_t index_;
	Processor* processor_;
public:
	StackIndex(size_t index, Processor* processor, bool isGlobal = false);
	StackIndex(Element element, Processor* processor);
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

class Instruction;

class Function
{
	FunctionType type_;
	std::vector<Instruction> body_;
	Processor* processor_;
public:
	Function() : type_(), body_(), processor_(nullptr) {}
	Function(FunctionType type, const std::vector<Instruction>& body, Processor* processor) : type_(type), body_(body), processor_(processor) {}
	Function(const Function& other) : type_(other.type_), body_(other.body_), processor_(other.processor_) {}
	Function(Function&& other) : type_(std::move(other.type_)), body_(std::move(other.body_)), processor_(other.processor_) {}

	Function(StackIndex index, Processor* processor);

	Function& operator=(const Function& other);

	Function& operator=(Function&& other);

	bool isValid() const;



	FunctionType type() const { return type_; }
	const std::vector<Instruction>& body() const { return body_; }
};

typedef std::variant<int64_t, size_t, std::string, Condition, TypeVariant, Function, StackIndex, std::vector<Instruction>> Argument;



class Instruction
{
	OpCode opCode_;
	std::vector<Argument> arguments_;
public:
	Instruction(OpCode opCode, std::vector<Argument> arguments_ = {}) : opCode_(opCode), arguments_(arguments_) {}
	OpCode opCode() const { return opCode_; }
	const std::vector<Argument>& arguments() { return arguments_; }
};



class Processor
{
	friend class StackIndex;
	friend class Function;

	std::vector<Instruction> programm_;
	std::vector<BaseType> baseTypes_;
	std::vector<StructType> structs_;
	Stack stack_;
	std::vector<size_t> FunctionStackStartPositions_;
	Stack FunctionReturnValues_;
	bool finished_;

	std::optional<int64_t> execute(Instruction instruction);
	public:
	Processor(std::vector<Instruction> program);
	std::optional<int64_t> run();
	void notifyStackReallocation(uint8_t* new_data);

	bool finished();
};

#endif