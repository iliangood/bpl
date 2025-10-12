#if !defined PROCESSOR_H
#define PROCESSOR_H

#include <vector>
#include <string>
#include <variant>

#include "variables/stack.h"
#include "variables/type.h"


enum class OpCode
{
	end = 0,
	call,
	ret,
	init,
	newLevel,
	popLevel,
	jmp,
	cmp,
	jpt,
	add,
	sub,
	mul,
	div 
};



typedef  std::variant<std::string, size_t> ArgumentType;



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

	void execute(Instruction instruction);
	public:
	Processor(std::vector<Instruction> program);
	void run();

	bool finished();
};

#endif