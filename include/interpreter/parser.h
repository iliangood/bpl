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

class Scope
{
	std::unordered_map<std::string, size_t> variables_;
public:
	Scope() {}
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

class parser
{
	Processor* processor_;
	
	Scope globalScope_;
	std::vector<Scope> scopes_;

	Instruction ParseLine();
public:
	parser(Processor processor);
	
	std::vector<Instruction> parse(std::string_view programm);
};



#endif