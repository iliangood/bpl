#if !defined PARSER_H
#define PARSER_H

#include <string>
#include <string_view>

#include <unordered_map>

#include "processor.h"

class Variable
{
	std::string name_;
	size_t offset_;
public:
	Variable(std::string_view name, size_t offset) : name_(name), offset_(offset) {}
	Variable(std::string&& name, size_t offset) : name_(std::move(name)), offset_(offset) {}
	size_t offset() const { return offset_; }
	std::string& name() { return name_; }
	const std::string& name() const { return name_; }
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
	size_t offset(std::string name)
	{
		return variables_[name];
	}
	size_t get(std::string name)
	{
		return variables_[name];
	}
};

class FunctionScope
{
	std::vector<BlockScope> scopes_;
public:
	void inScope()
	{
		scopes_.emplace_back();
	}
	void outScope()
	{
		scopes_.pop_back();
	}
	std::vector<BlockScope>& scopes();
};

class Parser
{
	Processor* processor_;
	
	FunctionScope globalScope_;
	std::vector<FunctionScope> scopes_;

	Instruction ParseInstruction();
public:
	Parser(Processor processor);
	
	std::vector<Instruction> parse(std::string_view programm);
};



#endif