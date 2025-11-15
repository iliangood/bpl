#include "interpreter/parser.h"
#include <sstream>

Parser::Parser(Processor* processor)
	: processor_(processor), globalScope_(), scopes_()
{}

std::optional<Argument> Parser::parseArgument(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end)
{
	if(*it == end)
		throw std::runtime_error("Unexpected end of program while parsing argument");
	Argument arg;
	std::stringstream ss(**it);
	std::vector<std::string> parts;
	std::string part;
	while(std::getline(ss, part, ':'))
	{
		parts.push_back(part);
	}
	if(parts.size() == 0)
		throw std::runtime_error("Invalid argument format: " + **it);
	if(parts[0] == "Value")
	{
		if(parts[1] == "Function" && parts.size() >= 3)
		{
			Function func;
			std::vector<Instruction> body;
			FunctionType funcType;
			
			++(*it);
			while(**it != ")")
			{
			}
			
			return arg;
		}
		if(parts.size() != 3)
			throw std::runtime_error("Invalid Value argument format: " + **it);
		if(parts[1] == "int64")
		{
			int64_t value = std::stoll(parts[2]);
			arg = Value(value);
		}
		else if(parts[1] == "char")
		{
			if(parts[2].size() != 1)
				throw std::runtime_error("Invalid char Value argument format: " + **it);
			char value = parts[2][0];
			arg = Value(value);
		}
		else
		{
			throw std::runtime_error("Unknown Value type: " + parts[1]);
		}
		++(*it);
		return arg;
	}
}

Instruction Parser::ParseInstruction(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end)
{
	OpCode opCode;
	std::vector<Argument> arguments;
	if(*it == end)
		throw std::runtime_error("Unexpected end of program while parsing instruction");
	std::optional<OpCode> opCodeOpt = parseOpcode(**it);
	if(!opCodeOpt.has_value())
		throw std::runtime_error("Unknown opcode: " + **it);
	opCode = opCodeOpt.value();
	++(*it);

}