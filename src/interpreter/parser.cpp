#include "interpreter/parser.h"

Parser::Parser(Processor processor)
	: processor_(&processor), globalScope_(), scopes_()
{}

std::optional<Argument> Parser::parseArgument(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end)
{
	if(*it == end)
		throw std::runtime_error("Unexpected end of program while parsing argument");
	Argument arg;
	if((*it)->starts_with("value"))
	{

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