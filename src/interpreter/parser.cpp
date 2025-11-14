#include "interpreter/parser.h"

Parser::Parser(Processor* processor)
{
	if(processor == nullptr)
		throw std::invalid_argument("Parser::Parser(Processor* processor) prcoessor is nullptr");
	processor_ = processor;
}

Instruction Parser::ParseInstruction()
{

}


std::vector<Instruction> Parser::parse(std::string_view programm)
{

}