#if !defined STACK_H
#define STACK_H

#include <cstddef>
#include <vector>
#include <string>
#include <optional>
#include <inttypes.h>
#include <cstring>

class ElementInfo
{
	std::string name_;
	size_t size_;
public:
	ElementInfo(std::string name, size_t size) : name_(name), size_(size) {}
	size_t size() const { return size_; }
	std::string name() const { return name_; }
};

class Element : public ElementInfo
{
	uint8_t* ptr_;
public:
	Element(ElementInfo info, uint8_t* ptr) : ElementInfo(info), ptr_(ptr) {}
	uint8_t* ptr() { return ptr_; }
	const uint8_t* ptr() const { return ptr_; }
};

class Stack
{
	std::vector<Element> elements_;
	uint8_t* data_;
	size_t top_;
	size_t capacity_;
	std::vector<size_t> levels_;
	bool cleanStackBeforeUse_;
	Stack(uint8_t* data, size_t capacity, bool cleanStackBeforeUse) : data_(data), top_(0), capacity_(capacity), cleanStackBeforeUse_(cleanStackBeforeUse) {};
	public:
	static std::optional<Stack> make(size_t capacity, bool cleanStackBeforeUse = false);
	~Stack();
	
	size_t capacity() const { return capacity_; }
	size_t top() const { return top_; }
	size_t size() const { return top_; }
	bool empty() const { return top_ == 0; }
	
	Element info(size_t index) const { return elements_.at(index); }
	Element infoFromEnd(size_t index) const { return elements_.at(top_ - 1 - index); }
	
	std::optional<uint8_t*> push(const ElementInfo& element);
	void pop();
	void pop(size_t count);

	void setCleanStackBeforeUse(bool clean) { cleanStackBeforeUse_ = clean; }
	bool cleanStackBeforeUse() const { return cleanStackBeforeUse_; }
	
	void popToLevel(size_t level);
	size_t currentLevel() const { return levels_.size(); }

	uint8_t* at(size_t index);
	const uint8_t* at(size_t index) const;


	uint8_t* operator[](size_t index) { return at(index); }
	const uint8_t* operator[](size_t index) const { return at(index);}

	uint8_t* atFromEnd(size_t index);
	const uint8_t* atFromEnd(size_t index) const;

	int resize(size_t new_capacity);
	void clear();

	int newLevel();
	int deleteLevel();


};

#endif