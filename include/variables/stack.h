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
	size_t elementCount() const { return type_.elementCount(); }
};

class Element : public ElementInfo
{
	friend class Stack;
	size_t pos_;
	size_t index_;
	void setIndex(size_t index) { index_ = index; }
public:
	Element(ElementInfo info, size_t pos) : ElementInfo(info), pos_(pos) {}
	size_t pos() const { return pos_; }
	size_t index() const { return index_; }

	Element at(size_t index) 
	{
		size_t pos = elements_.size()/2;
		size_t step = pos/2;
		std::vector<Element> elements = 
	}
	Element atFromEnd(size_t index) const;

	Element operator[](size_t index) const { return at(index); }
};

class Stack
{
	std::vector<Element> elements_;
	uint8_t* data_;
	size_t top_;
	std::vector<size_t> levels_;
	Processor* processor_;
	size_t capacity_;
	size_t elementCounter_;
	bool cleanStackBeforeUse_;
public:
	Stack(Processor* processor, size_t capacity = 1 << 20, bool cleanStackBeforeUse = false);
	
	~Stack();
	
	size_t capacity() const { return capacity_; }
	size_t top() const { return top_; }
	size_t size() const { return top_; }
	bool empty() const { return top_ == 0; }
	
	std::optional<Element> element(size_t index) const;
	std::optional<Element> elementFromEnd(size_t index) const;

	std::optional<size_t> find(std::string name);

	std::vector<Element>& elements() { return elements_; }
	size_t elementCount() { return elementCounter_; }
	
	uint8_t* push(const ElementInfo& element, bool initAfterPush = false);
	uint8_t* push(const Element& element);
	void pop();
	void pop(size_t count);

	void setCleanStackBeforeUse(bool clean) { cleanStackBeforeUse_ = clean; }
	bool cleanStackBeforeUse() const { return cleanStackBeforeUse_; }
	
	size_t currentLevel() const { return levels_.size(); }
	void newLevel();
	void popLevel();
	void deleteLevel() { popLevel(); return; }
	void popToLevel(size_t level);

	std::optional<uint8_t*> at(size_t index);
	std::optional<const uint8_t*> at(size_t index) const;

	std::optional<uint8_t*> operator[](size_t index) { return at(index); }
	std::optional<const uint8_t*> operator[](size_t index) const { return at(index);}

	std::optional<uint8_t*> atFromEnd(size_t index);
	std::optional<const uint8_t*> atFromEnd(size_t index) const;

	void resize(size_t new_capacity);
	void clear();

};

#endif