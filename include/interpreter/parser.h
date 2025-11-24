#if !defined PARSER_H
#define PARSER_H

#include <string>
#include <string_view>

#include <unordered_map>

#include "processor.h"

class Variable
{
	TypeVariant type_;
	PreStackIndex index_;
public:
	Variable(const TypeVariant& type, PreStackIndex index) : type_(type), index_(index) {}
	const PreStackIndex& index() const { return index_; }
	PreStackIndex& index() { return index_; }
	const TypeVariant& type() const { return type_; }
	TypeVariant& type() { return type_; }
};

class BlockScope
{
	std::unordered_map<std::string, Variable> variables_;
public:
	BlockScope() {}
	void insert(const Variable& variable, std::string_view name)
	{
		variables_.insert({std::string(name), variable});
	}
	std::optional<Variable> find(std::string name) const
	{
		std::unordered_map<std::string, Variable>::const_iterator it = variables_.find(name);
		if(variables_.end() == it)
			return std::nullopt;
		return it->second;
	}
};

class FunctionScope
{
	std::vector<BlockScope> scopes_;
public:
	FunctionScope()
	{
		scopes_.emplace_back();
	}
	void inScope()
	{
		scopes_.emplace_back();
	}
	void outScope()
	{
		scopes_.pop_back();
	}
	void insert(const Variable& variable, std::string_view name)
	{
		scopes_.back().insert(variable, name);
	}
	std::vector<BlockScope>& scopes()
	{
		return scopes_;
	}
	const std::vector<BlockScope>& scopes() const
	{
		return scopes_;
	}
	std::optional<Variable> find(std::string_view name) const 
	{
		for(std::vector<BlockScope>::const_reverse_iterator it = scopes_.crbegin(); it != scopes_.crend(); ++it)
		{
			std::optional<Variable> jt = it->find(std::string(name));
			if(jt.has_value())
				return jt.value();
		}
		return std::nullopt;
	}
};

class Parser
{
	Processor* processor_;
	
	std::vector<FunctionScope> scopes_;
	std::vector<size_t> currentFunctionOffsets_;

	std::optional<Variable> findVariable(const std::string& name) const;

	std::optional<Argument> parseArgument(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end);
	std::vector<Argument> parseArguments(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end);

	Instruction parseInstruction(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end);
	TypeVariant parseType(const std::string& typeStr) const;
public:
	Parser(Processor* processor);
	
	std::vector<Instruction> parse(std::string_view programm);
	std::vector<Instruction> parse(const std::vector<std::string>& programm);
};



#endif