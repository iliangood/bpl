#include "interpreter/parser.h"
#include <sstream>
#include <ranges>

std::vector<std::string> split(std::string_view str, char delimiter)
{
    std::vector<std::string> parts;
    for (auto part : std::ranges::split_view(str, delimiter)) {
        parts.emplace_back(part.begin(), part.end());
    }
    return parts;
}

Parser::Parser(Processor* processor)
	: processor_(processor), scopes_()
{}

std::optional<Variable> Parser::findVariable(const std::string& name) const
{
	const FunctionScope& currentScope = scopes_.back();
	std::optional<Variable> varOpt = currentScope.find(name);
	if(varOpt.has_value())
		return varOpt.value();
	const FunctionScope& globalScope = scopes_.front();
	varOpt = globalScope.find(name);
	if(varOpt.has_value())
		return varOpt.value();
	return std::nullopt;
}

std::optional<Argument> Parser::parseArgument(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end) //TODO:
{
	if(*it == end)
		throw std::runtime_error("Unexpected end of program while parsing argument");
	Argument arg;
	std::vector<std::string> parts = split(**it, ':');
	if(parts.size() == 0)
		throw std::runtime_error("Invalid argument format: " + **it);
	if(parts[0] == "value")
	{
		if(parts[1] == "function" && parts.size() >= 3)
		{
			scopes_.emplace_back();
			Function func;
			std::vector<Instruction>& body = func.body();
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
			while(**it != "endFunction")
			{
				std::optional<Instruction> instrOpt = parseInstruction(it, end);
				if(!instrOpt.has_value())
					throw std::runtime_error("Unexpected end of program while parsing Function Value argument body");
				body.push_back(instrOpt.value());
				++(*it);
			}
			++(*it);
			scopes_.pop_back();
			arg = func;
			
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
	else if(parts[0] == "type")
	{
		std::optional<TypeVariant> typeOpt = processor_->typeByName(parts[1]);
		if(!typeOpt.has_value())
			throw std::runtime_error("Unknown type in Type argument: " + parts[1]);
		TypeVariant type = typeOpt.value();
		for(size_t i = 2; i < parts.size(); ++i)
		{
			if(parts[i].empty())
				throw std::runtime_error("Empty type name in Type argument: " + **it);
			if(parts[i] == "ptr")
			{
				type = PointerType(typeOpt.value());
				if(!type.isValid())
					throw std::runtime_error("Invalid PointerType in Type argument: " + **it);
			}
			else if(parts[i] == "link")
			{
				type = LinkType(typeOpt.value());
				if(!type.isValid())
					throw std::runtime_error("Invalid LinkType in Type argument: " + **it);
			}
			else if(parts[i].starts_with("array"))
			{
				size_t pos = parts[i].find('[');
				size_t pos2 = parts[i].find(']');
				if(pos == std::string::npos || pos2 == std::string::npos || pos2 <= pos + 1)
					throw std::runtime_error("Invalid ArrayType format in Type argument: " + parts[i]);
				std::string countStr = parts[i].substr(pos + 1, pos2 - pos - 1);
				size_t count = std::stoull(countStr);
				type = ArrayType(typeOpt.value(), count);
				if(!type.isValid())
					throw std::runtime_error("Invalid ArrayType in Type argument: " + **it);
			}
			else
			{
				throw std::runtime_error("Unknown type modifier in Type argument: " + parts[i]);
			}
			
				
		}
		
		arg = typeOpt.value();
		++(*it);
		return arg;
	}
	else if(parts[0] == "variable")
	{
		if(parts.size() != 2)
			throw std::runtime_error("Invalid PreStackIndex argument format: " + **it);
		std::optional<Variable> varOpt = findVariable(parts[1]);
		if(!varOpt.has_value())
			throw std::runtime_error("Unknown variable name in PreStackIndex argument: " + parts[2]);
		Variable var = varOpt.value();
		arg = var.index();
		++(*it);
		return arg;
	}
	else if(parts[0] == "instructions")
	{
		++(*it);
		std::vector<Instruction> instructions;
		scopes_.back().inScope();
		while(*it != end && **it != "endInstructions")
		{
			std::optional<Instruction> instrOpt = parseInstruction(it, end);
			if(!instrOpt.has_value())
				throw std::runtime_error("Unexpected end of program while parsing Instructions argument");
			instructions.push_back(instrOpt.value());
			++(*it);
		}
		if(*it == end)
			throw std::runtime_error("Unexpected end of program while parsing Instructions argument");
		++(*it); 
		arg = instructions;
		scopes_.back().outScope();
		return arg;
	}
	else
	{
		throw std::runtime_error("Unknown argument type: " + parts[0]);
	}
}

std::vector<Argument> Parser::parseArguments(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end)
{
	std::vector<Argument> arguments;
	while(**it != ")")
	{
		std::optional<Argument> argOpt = parseArgument(it, end);
		if(!argOpt.has_value())
			throw std::runtime_error("Cannot parse argument: " + **it);
		arguments.push_back(argOpt.value());
		++(*it);
		if(*it == end)
			throw std::runtime_error("Unexpected end of program while parsing arguments");
	}
	return arguments;
}

Instruction Parser::parseInstruction(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end) //TODO:
{
	OpCode opCode;
	std::vector<Argument> arguments;
	std::vector<std::string> parts = split(**it, ':');
	if(parts.size() == 0)
		throw std::runtime_error("Invalid instruction format: " + **it);
	std::optional<OpCode> opCodeOpt = parseOpcode(parts[0]);
	if(!opCodeOpt.has_value())
		throw std::runtime_error("Unknown OpCode in instruction: " + parts[0]);
	opCode = opCodeOpt.value();
	if(opCode == OpCode::init_) // TODO:
	{
		if(parts.size() != 2)
			throw std::runtime_error("Invalid init instruction format: " + **it);
		std::string varName = parts[1];
		std::optional<Variable> varOpt = scopes_.back().find(varName);
		if(varOpt.has_value())
			throw std::runtime_error("Variable already declared in current scope: " + varName);
		Variable var = varOpt.value();
		++(*it);
		arguments = parseArguments(it, end);
		if(arguments.size() != 1)
			throw std::runtime_error("Invalid number of arguments for init instruction: " + std::to_string(arguments.size()));
		if(!std::holds_alternative<TypeVariant>(arguments[0]))
			throw std::runtime_error("Invalid argument type for init instruction, expected TypeVariant");
		TypeVariant varType = std::get<TypeVariant>(arguments[0]);

		if(!varIndex.has_value())
			throw std::runtime_error("Failed to get variable index after init");
		scopes_.back().insert(Variable(varName, varIndex.value().index()));
		return Instruction(opCode, arguments);
	}
}