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

class Instruction;

typedef std::variant<int64_t, std::string, Condition, size_t, std::vector<Instruction>> Argument;



class Instruction
{
	OpCode opCode_;
	std::vector<Argument> arguments_;
public:
	Instruction(OpCode opCode, std::vector<Argument> arguments_);
	OpCode opCode();
	std::vector<Argument> arguments();
	bool isValid();
};



class Processor
{
	std::vector<Instruction> programm_;
	std::vector<BaseType> baseTypes_;
	std::vector<Struct> structs_;
	Stack stack_;
	std::vector<size_t> FunctionStackStartPositions_;
	Stack FunctionReturnValues_;
	bool finished_;

	std::optional<int64_t> execute(Instruction instruction);
	public:
	Processor(std::vector<Instruction> program);
	std::optional<int64_t> run();

	bool finished();
};

#endif