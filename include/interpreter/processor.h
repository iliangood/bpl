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
	div_ 
};

class Instruction;

typedef std::variant<std::string, size_t, std::vector<Instruction>> ArgumentType;



class Instruction
{
	OpCode opCode_;
	std::vector<ArgumentType> arguments_;
public:
	Instruction(OpCode opCode, std::vector<ArgumentType> arguments_);
	OpCode opCode();
	std::vector<ArgumentType> arguments();
	bool isValid();
};



class Processor
{
	std::vector<Instruction> programm_;
	std::vector<BaseType> baseTypes_;
	std::vector<Struct> structs_;
	Stack stack_;
	Stack globalVariablesStack;
	bool finished_;

	std::optional<std::variant<int, std::string>> execute(Instruction instruction);
	public:
	Processor(std::vector<Instruction> program);
	void run();

	bool finished();
};

#endif