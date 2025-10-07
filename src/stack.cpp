#include "stack.h"

std::optional<Stack> Stack::make(size_t capacity)
{
	uint8_t* data = new uint8_t[capacity];
	return Stack(data, capacity);
}

Stack::~Stack()
{

}

int Stack::push(const ElementInfo& element)
{
	if (top_ + element.size() > capacity_)
	{
		if(resize((top_ + element.size()) * 2) != 0)
			return -1;
	}
	elements_.push_back(element);
	memcpy(data_ + top_, element.ptr(), element.size());
	top_ += element.size();
	return 0;
}
void Stack::pop()
{
	if (elements_.back().size() > top_)
		return;
	top_ -= elements_.back().size();
	elements_.pop_back();
}

uint8_t* Stack::at(size_t index)
{
	if(index >= elements_.size())
		return nullptr;
	size_t offset = 0;
	for(size_t i = 0; i < index; ++i)
		offset += elements_[i].size();
	return data_ + offset;
}
const uint8_t* Stack::at(size_t index) const;

uint8_t* Stack::atFromEnd(size_t index);
const uint8_t* Stack::atFromEnd(size_t index) const;

int Stack::resize(size_t new_capacity);
void Stack::clear();