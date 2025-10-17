#if !defined STACK_H
#define STACK_H

#include <cstddef>
#include <vector>
#include <string>
#include <optional>
#include <inttypes.h>
#include <cstring>
#include <stdexcept>

#include "variables/type.h"

class Processor;

class ElementInfo
{
	std::string name_;
	TypeVariant type_;
public:
	ElementInfo(std::string name, TypeVariant type) : name_(name), type_(type) {}
	size_t size() const { return sizeOfTypeVariant(type_); }
	TypeVariant type() const { return type_; }
	std::string name() const { return name_; }
};

class Element : public ElementInfo
{
	size_t pos_;
public:
	Element(ElementInfo info, size_t pos) : ElementInfo(info), pos_(pos) {}
	size_t pos() { return pos_; }
};

class Stack
{
	std::vector<Element> elements_;
	uint8_t* data_;
	size_t top_;
	size_t capacity_;
	std::vector<size_t> levels_;
	Processor* processor_;
	bool cleanStackBeforeUse_;
public:
	Stack(size_t capacity = 1 << 20, bool cleanStackBeforeUse = false);
	
	~Stack();
	
	size_t capacity() const { return capacity_; }
	size_t top() const { return top_; }
	size_t size() const { return top_; }
	bool empty() const { return top_ == 0; }
	
	Element element(size_t index) const { return elements_.at(index); }
	Element elementFromEnd(size_t index) const { return elements_.at(elements_.size() - 1 - index); }

	std::optional<size_t> find(std::string name);

	std::vector<Element>& elements() { return elements_; }
	size_t elementCount() { return elements_.size(); }
	
	uint8_t* push(const ElementInfo& element);
	uint8_t* push(const Element& element);
	void pop();
	void pop(size_t count);

	void setCleanStackBeforeUse(bool clean) { cleanStackBeforeUse_ = clean; }
	bool cleanStackBeforeUse() const { return cleanStackBeforeUse_; }
	
	size_t currentLevel() const { return levels_.size(); }
	int newLevel();
	int popLevel();
	int deleteLevel() { return popLevel(); }
	int popToLevel(size_t level);

	uint8_t* at(size_t index);
	const uint8_t* at(size_t index) const;

	uint8_t* operator[](size_t index) { return at(index); }
	const uint8_t* operator[](size_t index) const { return at(index);}

	uint8_t* atFromEnd(size_t index);
	const uint8_t* atFromEnd(size_t index) const;

	void resize(size_t new_capacity);
	void clear();

};

#endif