#include "variables/type.h"

#include "interpreter/processor.h"




BaseType::BaseType(std::string name, size_t size) : name_(std::move(name)), size_(size) 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!isValid())
			throw std::invalid_argument("Invalid BaseType parameters");
	}
}

size_t BaseType::size() const 
{
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("BaseType::size() called on invalid BaseType");
	}
	return size_; 
}

std::string BaseType::name() const 
{
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("BaseType::name() called on invalid BaseType");
	} 
	return name_; 
}

bool BaseType::isValid() const
{
	if(name_.empty())
		return false;	
	return true;
}



StructType::StructType(const std::vector<TypeVariant>& types, const std::vector<std::string>& fieldNames) : 
types_(types), fieldNames_(fieldNames), totalSize_(0)
{
	for (size_t i = 0; i < types_.size(); ++i)
	{
		totalSize_ += types_[i].size();
	}
	if(getValidationLevel() >= ValidationLevel::basic)
	{
		if(!isValid())
			throw std::invalid_argument("StructType::StructType(std::string, const std::vector<TypeVariant>&) Invalid parameters");
	}
}

size_t StructType::size() const 
{ 
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("StructType::size() called on invalid StructType");
	}
	return totalSize_; 
}

const std::vector<TypeVariant>& StructType::types() const 
{
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("StructType::baseTypes() called on invalid StructType");
	}
	return types_; 
}

TypeVariant StructType::type(size_t index) const
{
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("StructType::baseTypes() called on invalid StructType");
	}
	if(index == 0)
		return this;
	if(index > types_.size())
		throw std::out_of_range("StructType::elementSubIndex(size_t) index out of range");
	return types_[index-1];
}

bool StructType::operator==(const StructType& other) const
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("StructType::operator==(const StructType&) invalid other StructType");
	}
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("StructType::operator==(const StructType&) called on invalid StructType");
	}
	return types_ == other.types_;
}

bool StructType::isValid() const
{
	if(types_.empty())
		return false;
	if(getValidationLevel() < ValidationLevel::basic)
		return true;
	for(size_t i = 0; i < types_.size(); ++i)
	{
		if(!types_[i].isValid())
			return false;
	}
	return true;
}

std::vector<size_t> StructType::elementSubIndexes() const
{
	std::vector<size_t> subIndexes;
	subIndexes.reserve(types_.size());
	subIndexes.push_back(0);
	size_t top = 1;
	for(size_t i = 0; i < types_.size(); ++i)
	{
		subIndexes.push_back(top);
		top += types_[i].elementCount();
	}
	return subIndexes;
}

size_t StructType::elementSubIndex(size_t index) const
{
	if(index == 0)
		return 0;
	if(index > types_.size())
		throw std::out_of_range("StructType::elementSubIndex(size_t) index out of range");
	size_t top = 1;
	for(size_t i = 0; i < index - 1; ++i)
	{
		top += types_[i].elementCount();
	}
	return top;
}

std::vector<size_t> StructType::offsetsBySize() const
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!isValid())
			throw std::runtime_error("StructType::offsetsBySize(size_t) called on invalid StructType");
	}
	std::vector<size_t> offsets;
	offsets.reserve(types_.size() + 1);
	size_t currentOffset = 0;
	offsets.push_back(0);
	for(size_t i = 0; i < types_.size(); ++i)
	{
		offsets.push_back(currentOffset);
		currentOffset += types_[i].size();
	}
	return offsets;
}

size_t StructType::offsetBySize(size_t index) const
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!isValid())
			throw std::runtime_error("StructType::offsetsBySize(size_t) called on invalid StructType");
	}
	if(index > types_.size())
		throw std::out_of_range("StructType::offsetsBySize(size_t) index out of range");
	if(index == 0)
		return 0;
	size_t offset = 0;
	for(size_t i = 0; i < index-1; ++i)
	{
		offset += types_[i].size();
	}
	return offset;
}



PointerType::PointerType(TypeVariant pointerType) : pointerType_(new TypeVariant)
{
	*pointerType_ = pointerType;
	if(getValidationLevel() >= ValidationLevel::basic)
	{
		if(!isValid())
			throw std::invalid_argument("PointerType::PointerType(TypeVariant) Invalid PointerType parameters");
	}
}

TypeVariant PointerType::pointerType() const 
{
	if(pointerType_ == nullptr)
		throw std::runtime_error("PointerType::pointerType() called on null pointerType_");
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("PointerType::pointerType() called on invalid PointerType");
	}
	return *pointerType_; 
}

PointerType::PointerType(const PointerType& other) : pointerType_(new TypeVariant()) 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid PointerType::PointerType(const PointerType&) parameters");
	}
	*pointerType_ = other.pointerType();
}

PointerType::PointerType(PointerType&& other) 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid PointerType::PointerType(PointerType&&) parameters");
	}
	pointerType_ = std::move(other.pointerType_);
}

PointerType& PointerType::operator=(const PointerType& other)
{
	if (this == &other)
		return *this;
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid PointerType::operator=(const PointerType&) parameters");
	}
	if(pointerType_ == nullptr)
		pointerType_ = std::make_unique<TypeVariant>();
	*pointerType_ = other.pointerType();
	
	return *this;
}

PointerType& PointerType::operator=(PointerType&& other)
{
	if (this == &other)
		return *this;
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid PointerType::operator=(PointerType&&) parameters");
	}
	pointerType_ = std::move(other.pointerType_);
	return *this;
}

bool PointerType::isValid() const
{
	if(pointerType_ == nullptr)
		return false;
	if(getValidationLevel() < ValidationLevel::basic)
		return true;
	return pointerType_->isValid();
}

bool PointerType::operator==(const PointerType& other) const
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("PointerType::operator==(const PointerType&) invalid other PointerType");
	}
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("PointerType::operator==(const PointerType&) called on invalid PointerType");
	}
	return pointerType() == other.pointerType();
}



LinkType::LinkType(TypeVariant elementType)
{
	elementType_ = std::make_unique<TypeVariant>(elementType);
}

LinkType::LinkType()
{
	elementType_ = nullptr;
}

LinkType::LinkType(const LinkType& other)
{
	if(other.elementType_ == nullptr)
	{	
		elementType_ = nullptr;
		return;
	}
	elementType_ = std::make_unique<TypeVariant>(*other.elementType_);
}
LinkType::LinkType(LinkType&& other)
{
	elementType_ = std::move(other.elementType_);
}

LinkType LinkType::operator=(const LinkType& other)
{
	if(other.elementType_ == nullptr)
	{	
		elementType_ = nullptr;
		return *this;
	}
	elementType_ = std::make_unique<TypeVariant>(*other.elementType_);
	return *this;
}
LinkType LinkType::operator=(LinkType&& other)
{
	elementType_ = std::move(other.elementType_);
	return *this;
}

size_t LinkType::size() const
{
	return sizeof(Link);
}

std::optional<TypeVariant> LinkType::pointsTo() const
{
	if (!elementType_)
		return std::nullopt;
	return *elementType_;
}

bool LinkType::isValid() const
{
	if(elementType_ == nullptr)
		return true;
	return elementType_->isValid();
}



ArrayType::ArrayType(TypeVariant elementType, size_t count) : elementType_(new TypeVariant), count_(count) 
{
	*elementType_ = elementType;
	if(getValidationLevel() >= ValidationLevel::basic)
	{
		if(!isValid())
			throw std::invalid_argument("ArrayType::ArrayType(TypeVariant, size_t) Invalid ArrayType parameters");
	}
}

TypeVariant ArrayType::elementType() const 
{
	if(elementType_ == nullptr)
		throw std::runtime_error("ArrayType::elementType() called on null elementType_");
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("ArrayType::elementType() called on invalid ArrayType");
	}
	return *elementType_;
}

size_t ArrayType::count() const 
{
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("ArrayType::count() called on invalid ArrayType");
	}
	return count_; 
}

size_t ArrayType::size() const 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!isValid())
			throw std::runtime_error("ArrayType::size() called on invalid ArrayType");
	}
	 return elementType().size() * count_;
}

ArrayType::ArrayType(ArrayType&& other) : count_(other.count_) 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid ArrayType::ArrayType(ArrayType&&) parameters");
	}
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("ArrayType::ArrayType(ArrayType&&) called on invalid ArrayType");
	}
	elementType_ = std::move(other.elementType_);
	other.count_ = 0;
}

ArrayType::ArrayType(const ArrayType& other) : elementType_(new TypeVariant), count_(other.count_) 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid ArrayType::ArrayType(const ArrayType&) parameters");
	}
	*elementType_ = other.elementType();
}

ArrayType& ArrayType::operator=(ArrayType&& other)
{
	if (this == &other)
		return *this;
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid ArrayType::operator=(ArrayType&&) parameters");
	}
	elementType_ = std::move(other.elementType_);
	count_ = other.count_;
	other.count_ = 0;
	other.elementType_ = nullptr;
	return *this;
}

ArrayType& ArrayType::operator=(const ArrayType& other)
{
	if (this == &other)
		return *this;
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid ArrayType::operator=(const ArrayType&) parameters");
	}
	if(elementType_ == nullptr)
		elementType_ = std::make_unique<TypeVariant>();
	*elementType_ = other.elementType();
	count_ = other.count_;
	return *this;
}

bool ArrayType::isValid() const
{
	if(elementType_ == nullptr)
		return false;
	if(count_ == 0)
		return false;
	if(getValidationLevel() < ValidationLevel::basic)
		return true;
	return elementType_->isValid();
}

bool ArrayType::operator==(const ArrayType& other) const
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!isValid() || !other.isValid())
			throw std::runtime_error("ArrayType::operator== called on invalid ArrayType");
	}
	return elementType() == other.elementType() && count_ == other.count_;
}

std::vector<size_t> ArrayType::elementSubIndexes() const
{
	std::vector<size_t> subIndexes;
	size_t elementSize = elementType_->elementCount();
	subIndexes.reserve(count_ + 1);
	subIndexes.push_back(0);
	size_t top = 1;
	for(size_t i = 0; i < count_; ++i)
	{
		subIndexes.push_back(i);
		top += elementSize;
	}
	return subIndexes;
}

size_t ArrayType::elementCount() const
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!isValid())
			throw std::runtime_error("ArrayType::elementCount() called on invalid ArrayType");
	}
	return elementType_->elementCount() * count_ + 1;
}



FunctionType::FunctionType(const std::vector<TypeVariant>& argumentsTypes, TypeVariant returnType) :
	argumentsTypes_(argumentsTypes), returnType_(std::make_unique<TypeVariant>(returnType))
{
	if(getValidationLevel() >= ValidationLevel::basic)
	{
		if(!isValid())
			throw std::invalid_argument("FunctionType::FunctionType(TypeVariant, const std::vector<TypeVariant>&) Invalid FunctionType parameters");
	}
}

FunctionType::FunctionType(FunctionType&& other) 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid FunctionType::FunctionType(FunctionType&&) parameters");
	}
	returnType_ = std::move(other.returnType_);
	argumentsTypes_ = std::move(other.argumentsTypes_);
}

FunctionType::FunctionType(const FunctionType& other) 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid FunctionType::FunctionType(const FunctionType&) parameters");
	}
	returnType_ = std::make_unique<TypeVariant>(*other.returnType_);
	argumentsTypes_ = other.argumentsTypes_;
}

FunctionType& FunctionType::operator=(const FunctionType& other)
{
	if (this == &other)
		return *this;

	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid FunctionType::operator=(const FunctionType&) parameters");
	}

	returnType_ =  std::make_unique<TypeVariant>(*other.returnType_);
	argumentsTypes_ = other.argumentsTypes_;
	return *this;
}

FunctionType& FunctionType::operator=(FunctionType&& other)
{
	if (this == &other)
		return *this;

	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid FunctionType::operator=(FunctionType&&) parameters");
	}

	returnType_ = std::move(other.returnType_);
	argumentsTypes_ = std::move(other.argumentsTypes_);
	return *this;
}

bool FunctionType::isValid() const
{
	if(returnType_ == nullptr)
		return false;
	//std::cout << "returnType_ check" << std::endl;
	if(!returnType_->isValid())
		return false;
	//std::cout << "returnType_ checked" << std::endl;
	if(getValidationLevel() < ValidationLevel::basic)
		return true;
	
	for(size_t i = 0; i < argumentsTypes_.size(); ++i)
	{
		if(!argumentsTypes_[i].isValid())
			return false;
	}
	return true;
}

TypeVariant& FunctionType::returnType()  
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!isValid())
			throw std::runtime_error("FunctionType::returnType() called on invalid FunctionType");
	}
	return *returnType_; 
}

const TypeVariant& FunctionType::returnType() const
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!isValid())
			throw std::runtime_error("FunctionType::returnType() called on invalid FunctionType");
	}
	return *returnType_; 
}

const std::vector<TypeVariant>& FunctionType::argumentsTypes() const 
{
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("std::vector<TypeVariant>& FunctionType::argumentsTypes() called on invalid FunctionType");
	}
	return argumentsTypes_; 
}

std::vector<TypeVariant>& FunctionType::argumentsTypes() 
{
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("std::vector<TypeVariant>& FunctionType::argumentsTypes() called on invalid FunctionType");
	}
	return argumentsTypes_; 
}

size_t FunctionType::size() const 
{
	return sizeof(std::vector<Instruction>*);
}

bool FunctionType::operator==(const FunctionType& other) const
{
	return returnType() == other.returnType() && argumentsTypes_ == other.argumentsTypes_;
}



bool TypeVariant::operator==(const TypeVariant& other) const
{
	if (index() != other.index())
		return false;
	if (isBaseType())
		return get<const BaseType*>() == other.get<const BaseType*>();
	if(isStructType())
		return std::get<const StructType*>(*this) == other.get<const StructType*>();
	else if (isFunctionType())
		return std::get<FunctionType>(*this) == other.get<FunctionType>();
	else if (isPointerType())
		return std::get<PointerType>(*this) == other.get<PointerType>();
	else if (isArrayType())
		return std::get<ArrayType>(*this) == other.get<ArrayType>();
	return false;
}

bool TypeVariant::operator!=(const TypeVariant& other) const
{
	return !(*this == other);
}

bool TypeVariant::isValid() const
{
	if(isBaseType())
	{
		if(std::get<const BaseType*>(*this) == nullptr)
			return false;
		return std::get<const BaseType*>(*this)->isValid();
	}
	else if(isStructType())
	{
		if(std::get<const StructType*>(*this) == nullptr)
			return false;
		return std::get<const StructType*>(*this)->isValid();
	}
	else if(isPointerType())
		return std::get<PointerType>(*this).isValid();
	else if(isFunctionType())
	{
		return std::get<FunctionType>(*this).isValid();
	}
	else if(isArrayType())
	{
		return std::get<ArrayType>(*this).isValid();
	}
	else if(isLinkType())
	{
		return std::get<LinkType>(*this).isValid();
	}
	throw std::runtime_error("TypeVariant::isValid() called on unknown TypeVariant type");
	return false;
}

bool TypeVariant::isBaseType() const
{
	return std::holds_alternative<const BaseType*>(*this);
}
bool TypeVariant::isStructType() const
{
	return std::holds_alternative<const StructType*>(*this);
}
bool TypeVariant::isPointerType() const
{
	return std::holds_alternative<PointerType>(*this);
}
bool TypeVariant::isFunctionType() const
{
	return std::holds_alternative<FunctionType>(*this);
}
bool TypeVariant::isArrayType() const
{
	return std::holds_alternative<ArrayType>(*this);
}
bool TypeVariant::isLinkType() const
{
	return std::holds_alternative<LinkType>(*this);
}
size_t TypeVariant::size() const
{
	if (isBaseType())
		return std::get<const BaseType*>(*this)->size();
	if(isStructType())
		return std::get<const StructType*>(*this)->size();
	else if (isFunctionType())
		return std::get<FunctionType>(*this).size();
	else if (isPointerType())
		return std::get<PointerType>(*this).size();
	else if (isArrayType())
		return std::get<ArrayType>(*this).size();
	else if (isLinkType())
		return std::get<LinkType>(*this).size();
	throw std::runtime_error("TypeVariant::size() called on unknown TypeVariant type");
	return 0;
}
size_t TypeVariant::elementCount() const
{
	if (isBaseType())
		return get<const BaseType*>()->elementCount();
	if(isStructType())
		return get<const StructType*>()->elementCount();
	else if (isFunctionType())
		return get<FunctionType>().elemetCount();
	else if (isPointerType())
		return get<PointerType>().elementCount();
	else if (isArrayType())
		return get<ArrayType>().elementCount();
	else if (isLinkType())
		return get<LinkType>().elementCount();
	throw std::runtime_error("TypeVariant::elementCount() called on unknown TypeVariant type");
	return 0;
}