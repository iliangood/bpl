#include "interpreter/parser.h"
#include <sstream>
#include <ranges>
#include <algorithm>

std::string baseTrim(std::string_view str)
{
	size_t start = std::find_if_not(str.begin(), str.end(), [](unsigned char c){ return std::isspace(c); }) - str.begin();
	size_t end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char c){ return std::isspace(c); }).base() - str.begin();
	return std::string(str.substr(start, end - start));
}

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
	{
		Variable& var = varOpt.value();
		var.index().global() = true;
		return var;
	}
	return std::nullopt;
}

TypeVariant Parser::parseType(const std::string& name) const // TODO:
{
	size_t end = name.find_first_of("*[&(");
	std::string baseTypeName = name.substr(0, end);
	if(name[end] == '(')
	{
		TypeVariant returnType = parseType(baseTypeName);
		std::vector<TypeVariant> argTypes;
		std::vector<std::string> argTypeNames = split(name.substr(end + 1, name.find(')', end) - end - 1), ',');
		for(const std::string& argTypeName : argTypeNames)
		{
			std::optional<TypeVariant> argTypeOpt = parseType(baseTrim(argTypeName));
			if(!argTypeOpt.has_value())
				throw std::runtime_error("Unknown argument type in FunctionType: " + argTypeName);
			argTypes.push_back(argTypeOpt.value());
		}
		return FunctionType(std::move(argTypes), std::move(returnType));
	}
	std::optional<TypeVariant> typeOpt = processor_->typeByName(baseTypeName);
	if(!typeOpt.has_value())
		throw std::runtime_error("Unknown type name: " + baseTypeName);
	TypeVariant type = typeOpt.value();
	size_t pos = end;
	while(pos < name.size())
	{
		if(name[pos] == '*')
		{
			type = PointerType(type);
			if(!type.isValid())
				throw std::runtime_error("Invalid PointerType for type name: " + name);
			++pos;
		}
		else if(name[pos] == '&')
		{
			type = LinkType(type);
			if(!type.isValid())
				throw std::runtime_error("Invalid LinkType for type name: " + name);
			++pos;
		}
		else if(name[pos] == '[')
		{
			size_t pos2 = name.find(']', pos);
			if(pos2 == std::string::npos || pos2 <= pos + 1)
				throw std::runtime_error("Invalid ArrayType format in type name: " + name);
			std::string countStr = name.substr(pos + 1, pos2 - pos - 1);
			size_t count = std::stoull(countStr);
			type = ArrayType(type, count);
			if(!type.isValid())
				throw std::runtime_error("Invalid ArrayType for type name: " + name);
			pos = pos2 + 1;
		}
		else
		{
			throw std::runtime_error("Unknown type modifier in type name: " + name);
		}
	}
	return type;
}

std::optional<Argument> Parser::parseArgument(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end) //TODO:
{
	if(*it == end)
		throw std::runtime_error("Unexpected end of program while parsing argument");
	Argument arg;
	std::vector<std::string> parts = split(**it, ':');
	if(parts.size() == 0)
		throw std::runtime_error("Invalid argument format: \"" + **it + "\"");
	if(parts[0] == "value")
	{
		if(parts[1] == "function" && parts.size() >= 3)
		{
			scopes_.emplace_back();

			std::vector<TypeVariant> localArgTypes;
			localArgTypes.reserve(parts.size() - 3);
			std::optional<TypeVariant> returnTypeOpt = parseType(parts[2]);
			if(!returnTypeOpt.has_value())
				throw std::runtime_error("Unknown return type in Function Value argument: " + parts[2]);
			for(size_t i = 3; i < parts.size(); ++i)
			{
				std::optional<TypeVariant> argTypeOpt = parseType(parts[i]);
				if(!argTypeOpt.has_value())
					throw std::runtime_error("Unknown argument type in Function Value argument: " + parts[i]);
				localArgTypes.push_back(argTypeOpt.value());
			}
			++(*it);

			Function func(FunctionType(localArgTypes, returnTypeOpt.value()), std::vector<Instruction>());
			std::vector<Instruction>& body = func.body();
			while(**it != "end")
			{
				std::optional<Instruction> instrOpt = parseInstruction(it, end);
				if(!instrOpt.has_value())
					throw std::runtime_error("Unexpected end of program while parsing Function Value argument body");
				body.push_back(instrOpt.value());
			}
			++(*it);
			scopes_.pop_back();

			arg = Value(std::move(func));
			
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
		else if(parts[1] == "double")
		{
			double value = std::stod(parts[2]);
			arg = Value(value);
		}
		else
		{
			throw std::runtime_error("Unknown Value type: \"" + parts[1] + "\"");
		}
		++(*it);
		return arg;
	}
	else if(parts[0] == "type")
	{
		if(parts.size() != 2)
			throw std::runtime_error("Invalid TypeVariant argument format: " + **it);
		TypeVariant arg = parseType(parts[1]);
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
		return std::nullopt;
	}
}

std::vector<Argument> Parser::parseArguments(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end)
{
	std::vector<Argument> arguments;
	while(*it != end)
	{
		//std::cout << **it << std::endl;
		std::optional<Argument> argOpt = parseArgument(it, end);
		if(!argOpt.has_value())
			return arguments;
		arguments.push_back(argOpt.value());
		//std::cout << **it << std::endl;
	}
	return arguments;
}

Instruction Parser::parseInstruction(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end) //TODO: check errors
{
	//std::cout << **it << std::endl;
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
		++(*it);
		arguments = parseArguments(it, end);
		if(arguments.size() != 1)
			throw std::runtime_error("Invalid number of arguments for init instruction: " + std::to_string(arguments.size()));
		if(!std::holds_alternative<TypeVariant>(arguments[0]))
			throw std::runtime_error("Invalid argument type for init instruction, expected TypeVariant");
		TypeVariant varType = std::get<TypeVariant>(arguments[0]);

		size_t varOffset = currentFunctionOffsets_.back();
		currentFunctionOffsets_.back() += varType.elementCount();
		scopes_.back().insert(Variable(varType, PreStackIndex(varOffset)), varName);
		return Instruction(opCode, arguments);
	}
	++(*it);
	arguments = parseArguments(it, end);
	return Instruction(opCode, arguments);
}

std::string trim(std::string&& s)
{
    if (s.empty()) return s;

    auto is_space = [](unsigned char c) { return std::isspace(c); };

    std::string::iterator first = std::find_if_not(s.begin(), s.end(), is_space);
    if (first == s.end())
        return {};

    std::string::iterator last = std::find_if_not(s.rbegin(), s.rend(), is_space).base();
	if(last != s.end())
	{
		if(*(last-1) == ':')
			++last;
	}


    return s.substr(std::distance(s.begin(), first), std::distance(first, last));
}

std::vector<Instruction> Parser::parse(std::string_view programm) //TODO:
{
	std::vector<Instruction> instructions;
	std::vector<std::string> lines = split(programm, '\n');
	for(size_t i = 0, j = 0; i < lines.size(); ++i , ++j)
	{
		std::string line = trim(std::move(lines[i]));
		if(line.empty())
		{
			--j;
			continue;
		}
		lines[j] = line;
	}
	scopes_.emplace_back();
	currentFunctionOffsets_.push_back(0);
	std::vector<std::string>::const_iterator it = lines.cbegin();
	while(it != lines.cend())
	{
		std::optional<Instruction> instrOpt = parseInstruction(&it, lines.cend());
		if(!instrOpt.has_value())
			throw std::runtime_error("Cannot parse instruction: " + *it);
		instructions.push_back(instrOpt.value());
	}
	return instructions;
	
}

std::vector<Instruction> Parser::parse(const std::vector<std::string>& programm)
{
	std::vector<std::string> lines;
	lines.reserve(programm.size());
	for(size_t i = 0; i < programm.size(); ++i)
	{
		std::string line = trim(std::string(programm[i]));
		if(line.empty())
			continue;
		lines.push_back(line);
	}
	std::vector<Instruction> instructions;
	scopes_.emplace_back();
	currentFunctionOffsets_.push_back(0);
	std::vector<std::string>::const_iterator it = lines.cbegin();
	while(it != lines.cend())
	{
		std::optional<Instruction> instrOpt = parseInstruction(&it, lines.cend());
		if(!instrOpt.has_value())
			throw std::runtime_error("Cannot parse instruction: " + *it);
		instructions.push_back(instrOpt.value());
	}
	return instructions;
}