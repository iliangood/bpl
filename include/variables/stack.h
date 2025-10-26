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
	size_t size() const { return type_.size(); }
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
	Element(ElementInfo info, size_t pos, size_t index) : ElementInfo(info), pos_(pos), index_(index) {}
	size_t pos() const { return pos_; }
	size_t index() const { return index_; }

	std::optional<Element> at(size_t subIndex) const
	{
		if(subIndex == 0) 
			return *this;
		if(subIndex >= elementCount())
			return std::nullopt;
		TypeVariant typeV = type();
		if(typeV.isBaseType() || typeV.isPointerType() || typeV.isFunctionType() || typeV.isLinkType())
		{
			return std::nullopt;
		}
		else if(typeV.isStructType())
		{
			const StructType* structType = std::get<const StructType*>(typeV);
			size_t pos = structType->types().size()/2;
			size_t step = pos/2;
			std::vector<size_t> structSubIndexes = structType->elementSubIndexes();
			while(structSubIndexes[pos] != subIndex)
			{
				if(step == 0)
					step = 1;
				if(structSubIndexes[pos] > subIndex)
				{
					pos -= step;
				}
				else
				{
					if(structSubIndexes[pos] + structType->types()[pos].size() > subIndex)
					{
						Element element(ElementInfo("", structType->types()[pos]), pos_ + structType->offsetBySize(pos), index_ + structSubIndexes[pos]);
						return element.at(subIndex - structSubIndexes[pos]);
					}
					pos += step;
				}
				step /= 2;
			}
			return Element(ElementInfo("", structType->types()[pos]), pos_, subIndex);
		}
		else if(typeV.isArrayType())
		{
			const ArrayType& arrayType = std::get<ArrayType>(typeV);
			size_t elementSize = arrayType.elementType().elementCount();
			size_t arrayIndex = (subIndex - 1) / elementSize;
			size_t elementIndex = (subIndex - 1) % elementSize;
			if(arrayIndex >= arrayType.count())
				return std::nullopt;
			Element element(ElementInfo("", arrayType.elementType()), pos_ + arrayIndex * arrayType.elementType().size());
			return element.at(elementIndex);
		}
		throw std::runtime_error("Element::at(size_t) called on unsupported TypeVariant type");
	}

	std::optional<Element> atSubElements(size_t subIndex)
	{
		if(subIndex == 0)
			return *this;
		TypeVariant typeV = type();
		if(typeV.isBaseType() || typeV.isPointerType() || typeV.isFunctionType() || typeV.isLinkType())
		{
			return std::nullopt;
		}
		if(typeV.isStructType())
		{
			const StructType* structType = typeV.get<const StructType*>();
			return Element(ElementInfo("", structType->type(subIndex)), pos_ + structType->offsetBySize(subIndex) , index_ + structType->elementSubIndex(subIndex));
		}
		if(typeV.isArrayType())
		{
			ArrayType arrayType = typeV.get<ArrayType>();
			if(subIndex > arrayType.count())
				throw std::out_of_range("Element::atSubElements(size_t) out of range");
			TypeVariant elementType = arrayType.elementType();
			size_t elementSize = elementType.size();
			size_t elementsInElement = elementType.elementCount();
			return Element(ElementInfo("", elementType), pos_ + elementSize*(subIndex-1), index_ + 1 + elementsInElement*(subIndex-1));
		}
		throw std::runtime_error("Element::atSubElements(size_t) called on unsupported TypeVariant type");
	}

	std::optional<Element> atFromEnd(size_t index) const
	{
		size_t elemCount = elementCount();
		if(index >= elemCount)
			return std::nullopt;
		return at(elemCount - index - 1);
	}

	std::optional<Element> operator[](size_t index) const { return at(index); }
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

	std::optional<Element> wholeElement(size_t index) const;
	std::optional<Element> wholeElementFromEnd(size_t index) const;

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