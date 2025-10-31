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

	printCh_,
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


enum class BaseTypeId
{
	int64_ = 0,
	uint64_,
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



	FunctionType& type() { return type_; }
	const FunctionType& type() const { return type_; }
	std::vector<Instruction>& body() { return body_; }
	const std::vector<Instruction>& body() const { return body_; }
};

typedef std::variant<int64_t, uint64_t, std::string, void*, PreStackIndex, std::vector<Instruction>> Value;

typedef std::variant<Condition, TypeVariant, std::vector<Instruction>, Value> Argument;



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

	std::optional<int64_t> end_(Instruction& instruction);

	std::optional<int64_t> call_(Instruction& instruction);
	std::optional<int64_t> ret_(Instruction& instruction);

	std::optional<int64_t> scopeRet_(Instruction& instruction);
	std::optional<int64_t> init_(Instruction& instruction);
	std::optional<int64_t> mov_(Instruction& instruction);

	std::optional<int64_t> if_(Instruction& instruction);
	std::optional<int64_t> while_(Instruction& instruction);

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

	std::optional<int64_t> print_(Instruction& instruction);
	std::optional<int64_t> scan_(Instruction& instruction);

	std::optional<int64_t> cmp_(Instruction& instruction);
	

	std::optional<int64_t> execute(Instruction& instruction);

	bool returningFromFunction() const { return returningFromFunction_; }
	public:
	Processor(const std::vector<Instruction>& program, size_t stackSize = 1 << 20);
	std::optional<int64_t> run();
	void notifyStackReallocation(uint8_t* new_data);

	bool finished();
};

#endif