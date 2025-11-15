#include "interpreter/parser.h"
#include <sstream>

Parser::Parser(Processor* processor)
	: processor_(processor), globalScope_(), scopes_()
{}

std::optional<Argument> Parser::parseArgument(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end) //TODO:
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
		if(parts[1] == "Function" && parts.size() >= 3) //TODO:
		{
			Function func;
			std::vector<Instruction> body;
			FunctionType& funcType = func.type();
			std::vector<TypeVariant>& argTypes = funcType.argumentsTypes();
			std::optional<TypeVariant> returnTypeOpt = processor_->typeByName(parts[2]);
			if(!returnTypeOpt.has_value())
				throw std::runtime_error("Unknown return type in Function Value argument: " + parts[2]);
			funcType.returnType() = returnTypeOpt.value();
			for(size_t i = 3; i < parts.size(); ++i)
			{
				std::optional<TypeVariant> argTypeOpt = processor_->typeByName(parts[i]);
				if(!argTypeOpt.has_value())
					throw std::runtime_error("Unknown argument type in Function Value argument: " + parts[i]);
				argTypes.push_back(argTypeOpt.value());
			}
			++(*it);
			while(**it != "EndFunction")
			{
				std::optional<Instruction> instrOpt = parseInstruction(it, end);
				if(!instrOpt.has_value())
					throw std::runtime_error("Unexpected end of program while parsing Function Value argument body");
				body.push_back(instrOpt.value());
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
		else if(parts[1] == "bool")
		{
			if(parts[2] == "true")
			{
				arg = Value(bool(1));
			}
			else if(parts[2] == "false")
			{
				arg = Value(bool(0));
			}
			else if(parts[2] == "1")
			{
				arg = Value(bool(1));
			}
			else if(parts[2] == "0")
			{
				arg = Value(bool(0));
			}
			else
			{
				throw std::runtime_error("Invalid bool Value argument format: " + **it);
			}
		}
		if(parts[1] == "double")
		{
			double value = std::stod(parts[2]);
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

Instruction Parser::parseInstruction(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end)
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