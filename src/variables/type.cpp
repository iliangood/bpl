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
	if(size_ == 0)
	return false;

	if(getValidationLevel() < ValidationLevel::basic)
		return true;
	
	return true;
}



StructType::StructType(std::string name, const std::vector<TypeVariant>& types, const std::vector<std::string>& fieldNames) : name_(std::move(name)),
types_(types), fieldNames_(fieldNames), totalSize_(0)
{
	for (size_t i = 0; i < types_.size(); ++i)
	{
		totalSize_ += sizeOfTypeVariant(types_[i]);
	}
	if(getValidationLevel() >= ValidationLevel::basic)
	{
		if(!isValid())
			throw std::invalid_argument("StructType::StructType(std::string, const std::vector<TypeVariant>&) Invalid parameters");
	}
}

std::string StructType::name() const 
{
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("StructType::name() called on invalid StructType");
	}
	return name_; 
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
	if(name_.empty())
		return false;
	if(types_.empty())
		return false;
	if(getValidationLevel() < ValidationLevel::basic)
		return true;
	for(size_t i = 0; i < types_.size(); ++i)
	{
		if(!::isValid(types_[i]))
			return false;
	}
	return true;
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
	return ::isValid(*pointerType_);
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
	 return sizeOfTypeVariant(elementType()) * count_;
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
	return ::isValid(elementType_);
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

bool ArrayType::isTypeCompatible(const ArrayType& other) const
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!isValid() || !other.isValid())
			throw std::runtime_error("ArrayType::isTypeCompatible called on invalid ArrayType");
	}
	return elementType() == other.elementType();
}



FunctionType::FunctionType(TypeVariant returnType, const std::vector<TypeVariant>& argumentsTypes) :
	returnType_(new TypeVariant), argumentsTypes_(argumentsTypes) 
{
	*returnType_ = returnType;
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
	returnType_ = std::make_unique<TypeVariant>();
	*returnType_ = other.returnType();
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

	if(returnType_ == nullptr)
		returnType_ = std::make_unique<TypeVariant>();

	*returnType_ = other.returnType();
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
	if(getValidationLevel() < ValidationLevel::basic)
		return true;
	if(!::isValid(*returnType_))
		return false;
	for(size_t i = 0; i < argumentsTypes_.size(); ++i)
	{
		if(!::isValid(argumentsTypes_[i]))
			return false;
	}
	return true;
}

TypeVariant FunctionType::returnType() const 
{
	if(returnType_ == nullptr)
		throw std::runtime_error("FunctionType::returnType() called on invalid FunctionType");
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

size_t FunctionType::size() const 
{
	return sizeof(std::vector<Instruction>);
}

bool FunctionType::operator==(const FunctionType& other) const
{
	return returnType() == other.returnType() && argumentsTypes_ == other.argumentsTypes_;
}




bool isValid(const TypeVariant& var) 
{
	if(isBaseType(var))
	{
		if(std::get<const BaseType*>(var) == nullptr)
			return false;
		return std::get<const BaseType*>(var)->isValid();
	}
	else if(isStructType(var))
		{
			if(std::get<const StructType*>(var) == nullptr)
				return false;
			return std::get<const StructType*>(var)->isValid();
		}
	else if(isPointerType(var))
		return std::get<PointerType>(var).isValid();
	else if(isFunctionType(var))
	{
		return std::get<FunctionType>(var).isValid();
	}
	else if(isArrayType(var))
	{
		return std::get<ArrayType>(var).isValid();
	}
	else if(isStackLinkType(var))
	{
		return std::get<StackLinkType>(var).isValid();
	}
	return false;
}

bool isBaseType(const TypeVariant& var) 
{
	return std::holds_alternative<const BaseType*>(var);
}

bool isStructType(const TypeVariant& var) 
{
	return std::holds_alternative<const StructType*>(var);
}

bool isPointerType(const TypeVariant& var) 
{
	return std::holds_alternative<PointerType>(var);
}

bool isFunctionType(const TypeVariant& var) 
{
	return std::holds_alternative<FunctionType>(var);
}

bool isArrayType(const TypeVariant& var) 
{
	return std::holds_alternative<ArrayType>(var);
}

bool isStackLinkType(const TypeVariant& var) 
{
	return std::holds_alternative<StackLinkType>(var);
}

size_t sizeOfTypeVariant(const TypeVariant& var)
{
	if (isBaseType(var))
		return std::get<const BaseType*>(var)->size();
	else if (isStructType(var))
		return std::get<const StructType*>(var)->size();
	else if (isFunctionType(var))
		return std::get<FunctionType>(var).size();
	else if (isPointerType(var))
		return std::get<PointerType>(var).size();
	else if (isArrayType(var))
		return std::get<ArrayType>(var).size();
	else if (isStackLinkType(var))
		return std::get<StackLinkType>(var).size();
	return 0;
}

bool isBaseType(const std::unique_ptr<TypeVariant>& var) 
{
	if(var == nullptr)
		return false;
	return isBaseType(*var);
}

bool isStructType(const std::unique_ptr<TypeVariant>& var) 
{
	if(var == nullptr)
		return false;
	return isStructType(*var);
}

bool isPointerType(const std::unique_ptr<TypeVariant>& var) 
{
	if(var == nullptr)
		return false;
	return isPointerType(*var);
}

bool isFunctionType(const std::unique_ptr<TypeVariant>& var) 
{
	if(var == nullptr)
		return false;
	return isFunctionType(*var);
}

bool isArrayType(const std::unique_ptr<TypeVariant>& var) 
{
	if(var == nullptr)
		return false;
	return isArrayType(*var);
}

bool isStackLinkType(const std::unique_ptr<TypeVariant>& var) 
{
	if(var == nullptr)
		return false;
	return isStackLinkType(*var);
}

size_t sizeOfTypeVariant(const std::unique_ptr<TypeVariant>& var)
{
	if(var == nullptr)
		return 0;
	return sizeOfTypeVariant(*var);
}


bool operator==(const TypeVariant& a, const TypeVariant& b)
{
	if (a.index() != b.index())
		return false;
	if (isBaseType(a))
		return std::get<const BaseType*>(a) == std::get<const BaseType*>(b);
	else if (isStructType(a))
		return std::get<const StructType*>(a) == std::get<const StructType*>(b);
	else if (isFunctionType(a))
		return std::get<FunctionType>(a) == std::get<FunctionType>(b);
	else if (isPointerType(a))
		return std::get<PointerType>(a) == std::get<PointerType>(b);
	else if (isArrayType(a))
		return std::get<ArrayType>(a) == std::get<ArrayType>(b);
	return false;
}

bool operator!=(const TypeVariant& a, const TypeVariant& b)
{
	return !(a == b);
}

bool isTypeCompatible(const TypeVariant& a, const TypeVariant& b)
{
	if (a.index() != b.index())
		return false;
	if (isBaseType(a))
		return std::get<const BaseType*>(a) == std::get<const BaseType*>(b);
	else if (isStructType(a))
		return std::get<const StructType*>(a) == std::get<const StructType*>(b);
	else if (isFunctionType(a))
		return std::get<FunctionType>(a) == std::get<FunctionType>(b);
	else if (isPointerType(a))
		return std::get<PointerType>(a) == std::get<PointerType>(b);
	else if (isArrayType(a))
		return std::get<ArrayType>(a).isTypeCompatible(std::get<ArrayType>(b));
	return false;
}

bool isTypeCompatible(const TypeVariant* a, const TypeVariant* b)
{
	if (a == nullptr || b == nullptr)
		return false;
	return isTypeCompatible(*a, *b);
}

bool isTypeCompatible(const std::unique_ptr<TypeVariant>& a, const std::unique_ptr<TypeVariant>& b)
{
	if (a == nullptr || b == nullptr)
		return false;
	return isTypeCompatible(*a, *b);
}
