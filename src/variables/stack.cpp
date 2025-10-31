#include "variables/stack.h"
#include "interpreter/processor.h"

Stack::Stack(Processor* processor ,size_t capacity, bool cleanStackBeforeUse): top_(0), levels_({0}),
processor_(processor), capacity_(capacity), elementCounter_(0), cleanStackBeforeUse_(cleanStackBeforeUse)
{
	data_ = (uint8_t*)malloc(sizeof(uint8_t) * capacity);
	if(!data_)
		throw std::bad_alloc();
}

Stack::~Stack()
{
	free(data_);
}

uint8_t* Stack::push(const ElementInfo& element, bool initAfterPush)
{
	size_t elementSize = element.size();
	if (top_ + elementSize > capacity_)
		resize((top_ + element.size()) * 2);

	elements_.push_back(Element(element, top_, elementCounter_));
	if(cleanStackBeforeUse_ && !initAfterPush)
		memset(data_ + top_, 0, elementSize);
	top_ += elementSize;
	++levels_.back();
	elementCounter_ += element.elementCount();
	return data_ + top_ - elementSize;
}

uint8_t* Stack::push(const Element& element)
{
	uint8_t* dataPointer = push(static_cast<const ElementInfo&>(element), true);
	memcpy(dataPointer, data_ + element.pos(), element.size());
	return dataPointer;
}

void Stack::pop()
{
	if(elements_.empty())
		throw std::runtime_error("Stack::pop() Stack is empty");
	if(elements_.back().size() > top_)
		throw std::runtime_error("Stack::pop() Incorrect Stack: elements_.back().size() > top_");
	top_ -= elements_.back().size();
	elementCounter_ -= elements_.back().elementCount();
	elements_.pop_back();
	if(levels_.back() == 0)
		levels_.pop_back();
	--levels_.back();
}

void Stack::pop(size_t count)
{
	for(size_t i = 0; i < count; ++i)
		pop();
}

std::optional<Element> Stack::element(size_t index) const
{
	if(elements_.empty())
		return std::nullopt;
	if(index >= elementCounter_)
		return std::nullopt;
	size_t pos = elements_.size()/2;
	size_t step = pos/2;
	while(elements_[pos].index() != index)
	{
		if(elements_[pos].index() > index)
		{
			if(step == 0)
				step = 1;
			pos -= step;
		}
		else
		{
			if(elements_[pos].index() + elements_[pos].elementCount() > index)
			{
				return elements_[pos][index - elements_[pos].index()];
			}
			if(step == 0)
			{
				step = 1;
			}
			pos += step;
		}
		step /= 2;
	}
	return elements_[pos];
}

std::optional<Element> Stack::elementFromEnd(size_t index) const
{
	return element(elementCounter_ - 1 - index);
}

std::optional<uint8_t*> Stack::at(size_t index)
{
	std::optional<Element> elem = element(index);
	if(!elem.has_value())
		return std::nullopt;
	return data_ + elem->pos();
}

std::optional<const uint8_t*> Stack::at(size_t index) const
{
	std::optional<Element> elem = element(index);
	if(!elem.has_value())
		return std::nullopt;
	return data_ + elem->pos();
}

std::optional<uint8_t*> Stack::atWhole(size_t index)
{
	std::optional<Element> elem = wholeElement(index);
	if(!elem.has_value())
		return std::nullopt;
	return data_ + elem->pos();
}

std::optional<const uint8_t*> Stack::atWhole(size_t index) const
{
	std::optional<Element> elem = wholeElement(index);
	if(!elem.has_value())
		return std::nullopt;
	return data_ + elem->pos();
}

std::optional<uint8_t*> Stack::atWholeFromEnd(size_t index)
{
	return atWhole(elements_.size() - 1 - index);
}
std::optional<const uint8_t*> Stack::atWholeFromEnd(size_t index) const
{
	return atWhole(elements_.size() - 1 - index);
}

std::optional<uint8_t*> Stack::atFromEnd(size_t index)
{
	return at(elementCounter_ - 1 - index);
}

std::optional<const uint8_t*> Stack::atFromEnd(size_t index) const
{
	return at(elementCounter_ - 1 - index);
}

std::optional<Element> Stack::wholeElementFromEnd(size_t index) const
{
	return wholeElement(elements_.size() - 1 - index);
}

std::optional<Element> Stack::wholeElement(size_t index) const
{
	if(index > elements_.size())
		return std::nullopt;
	return elements_[index];
}

/*std::optional<size_t> Stack::find(std::string name)
{
	for(ssize_t i = elements_.size() - 1; i >= 0;++i)
	{
		if(elements_[i].name() == name)
			return i;
	}
	return std::nullopt;
}*/

void Stack::resize(size_t new_capacity)
{
	if(!getAllowResizeStack())
		throw std::runtime_error("Stack::resize(): stack overflow - resizing is forbidden yet, for use enable it via utils::setAllowResizeStack(true)");
	data_ = (uint8_t*)realloc(data_, new_capacity * sizeof(uint8_t));
	if(!data_)
		throw std::bad_alloc();
	capacity_ = new_capacity;
	processor_->notifyStackReallocation(data_);
}

void Stack::clear()
{
	elements_.clear();
	top_ = 0;
	levels_.clear();
}

void Stack::newLevel()
{
	levels_.push_back(0);
	return;
}

void Stack::popLevel()
{
	if(levels_.empty())
		throw std::runtime_error("Stack::popLevel() No level to pop");
	size_t count = levels_.back();
	pop(count);
	levels_.pop_back();
	if(count > elements_.size())
		throw std::runtime_error("Stack::popLevel() Incorrect Stack: count > elements_.size()");
	return;
}

void Stack::popToLevel(size_t level)
{
	if(level >= levels_.size())
		throw std::runtime_error("Stack::popToLevel() Level out of range");
	while(levels_.size() > level + 1)
	{
		popLevel();
	}
	return;
}