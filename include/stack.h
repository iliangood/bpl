#if !defined STACK_H
#define STACK_H

#include <cstddef>
#include <vector>
#include <string>

class ElementInfo
{
	std::string name_;
	size_t size_;
	void* ptr_;
public:
	ElementInfo(size_t size, void* ptr) : size_(size), ptr_(ptr) {}
	size_t size() const { return size_; }
	void* ptr() { return ptr_; }
	const void* ptr() const { return ptr_; }

};

class Stack
{
	std::vector<ElementInfo> elements_;
	unsigned char* data_;
	size_t top_;
	size_t capacity_;
public:
	Stack();
	~Stack();
	size_t capacity() const { return capacity_; }
	size_t top() const { return top_; }
	size_t size() const { return top_; }
	bool empty() const { return top_ == 0; }

	void* push(size_t size);
	void pop();

	unsigned char* at(size_t index);
	const unsigned char* at(size_t index) const;

	unsigned char* operator[](size_t index) { return at(index); }
	const unsigned char* operator[](size_t index) const { return at(index);}

	unsigned char* atFromEnd(size_t index);
	const unsigned char* atFromEnd(size_t index) const;

	int resize(size_t new_capacity);
	void clear();

};

#endif