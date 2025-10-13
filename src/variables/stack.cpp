#include "variables/stack.h"

Stack::Stack(size_t capacity, bool cleanStackBeforeUse): top_(0), capacity_(capacity), levels_({0}), cleanStackBeforeUse_(cleanStackBeforeUse)
{
	uint8_t* data = (uint8_t*)malloc(sizeof(uint8_t) * capacity);
	if(!data)
		throw std::bad_alloc();
}

Stack::~Stack()
{
	free(data_);
}

uint8_t* Stack::push(const ElementInfo& element)
{
	if (top_ + element.size() > capacity_)
		resize((top_ + element.size()) * 2);

	elements_.push_back(Element(element, top_));
	if(cleanStackBeforeUse_)
		memset(data_ + top_, 0, element.size());
	top_ += element.size();
	++levels_.back();
	return data_ + top_ - element.size();
}

void Stack::pop()
{
	if (elements_.back().size() > top_)
		throw std::runtime_error("Stack::pop() Incorrect Stack: elements_.back().size() > top_");
	top_ -= elements_.back().size();
	elements_.pop_back();
	--levels_.back();
	if(levels_.back() == 0 && levels_.size() > 1)
		levels_.pop_back();
}

void Stack::pop(size_t count)
{
	for(size_t i = 0; i < count; ++i)
		pop();
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

const uint8_t* Stack::at(size_t index) const
{
	if(index >= elements_.size())
		return nullptr;
	size_t offset = 0;
	for(size_t i = 0; i < index; ++i)
		offset += elements_[i].size();
	return data_ + offset;
}

uint8_t* Stack::atFromEnd(size_t index)
{
	if(index >= elements_.size())
		return nullptr;
	size_t offset = top_;
	for(size_t i = 0; i <= index; ++i)
		offset -= elements_[elements_.size() - 1 - i].size();
	return data_ + offset;
}

const uint8_t* Stack::atFromEnd(size_t index) const
{
	if(index >= elements_.size())
		return nullptr;
	size_t offset = top_;
	for(size_t i = 0; i <= index; ++i)
		offset -= elements_[elements_.size() - 1 - i].size();
	return data_ + offset;
}

std::optional<size_t> Stack::find(std::string name)
{
	for(size_t i = 0; i < elementCount(); ++i)
	{
		if(elementFromEnd(i).name() == name)
		{
			return i;
		}
	}
	return std::nullopt;
}

void Stack::resize(size_t new_capacity)
{
	data_ = (uint8_t*)realloc(data_, new_capacity * sizeof(uint8_t));
	if(!data_)
		throw std::bad_alloc();
	capacity_ = new_capacity;
}

void Stack::clear()
{
	elements_.clear();
	top_ = 0;
	levels_.clear();
}

int Stack::newLevel()
{
	levels_.push_back(0);
	return 0;
}

int Stack::popLevel()
{
	if(levels_.empty())
		return -1;
	size_t count = levels_.back();
	levels_.pop_back();
	if(count > elements_.size())
		return -1;
	pop(count);
	return 0;
}

int Stack::popToLevel(size_t level)
{
	if(level >= levels_.size())
		return -1;
	while(levels_.size() > level + 1)
	{
		if(popLevel() != 0)
			return -1;
	}
	return 0;
}