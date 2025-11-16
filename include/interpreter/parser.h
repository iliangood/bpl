#if !defined PARSER_H
#define PARSER_H

#include <string>
#include <string_view>

#include <unordered_map>

#include "processor.h"

class Variable
{
	std::string name_;
	TypeVariant type_;
	size_t offset_;
public:
	Variable(std::string_view name, const TypeVariant& type, size_t offset) : name_(name), type_(type), offset_(offset) {}
	Variable(std::string&& name, const TypeVariant& type, size_t offset) : name_(std::move(name)), type_(type), offset_(offset) {}
	size_t offset() const { return offset_; }
	std::string& name() { return name_; }
	const std::string& name() const { return name_; }
	const TypeVariant& type() const { return type_; }
	std::pair<std::string, size_t> toPair() const
	{
		return {name_, offset_};
	}
};

class BlockScope
{
	std::unordered_map<std::string, size_t> variables_;
public:
	BlockScope() {}
	void insert(const Variable& variable)
	{
		variables_.insert(variable.toPair());
	}
	std::optional<size_t> find(std::string name) const
	{
		std::unordered_map<std::string, size_t>::const_iterator it = variables_.find(name);
		if(variables_.end() == it)
			return std::nullopt;
		return it->second;
	}
	std::optional<size_t> get(std::string name) const
	{
		return find(name);
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
	void insert(const Variable& variable)
	{
		scopes_.back().insert(variable);
	}
	std::vector<BlockScope>& scopes()
	{
		return scopes_;
	}
	const std::vector<BlockScope>& scopes() const
	{
		return scopes_;
	}
	std::optional<size_t> find(std::string name) const 
	{
		for(std::vector<BlockScope>::const_reverse_iterator it = scopes_.crbegin(); it != scopes_.crend(); ++it)
		{
			std::optional<size_t> jt = it->find(name);
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

	std::optional<size_t> findVariable(const std::string& name) const;

	std::optional<Argument> parseArgument(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end);
	std::vector<Argument> parseArguments(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end);

	Instruction parseInstruction(std::vector<std::string>::const_iterator* it, const std::vector<std::string>::const_iterator& end);
public:
	Parser(Processor* processor);
	
	std::vector<Instruction> parse(std::string_view programm);
};



#endif