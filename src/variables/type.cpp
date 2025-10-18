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



Struct::Struct(std::string name, const std::vector<TypeVariant>& types) : name_(std::move(name)), types_(types), totalSize_(0)
{
	for (size_t i = 0; i < types_.size(); ++i)
	{
		totalSize_ += sizeOfTypeVariant(types_[i]);
	}
	if(getValidationLevel() >= ValidationLevel::basic)
	{
		if(!isValid())
			throw std::invalid_argument("Struct::Struct(std::string, const std::vector<TypeVariant>&) Invalid parameters");
	}
}

std::string Struct::name() const 
{
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("Struct::name() called on invalid Struct");
	}
	return name_; 
}

size_t Struct::size() const 
{ 
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("Struct::size() called on invalid Struct");
	}
	return totalSize_; 
}

const std::vector<TypeVariant>& Struct::baseTypes() const 
{
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("Struct::baseTypes() called on invalid Struct");
	}
	return types_; 
}

bool Struct::operator==(const Struct& other) const
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Struct::operator==(const Struct&) invalid other Struct");
	}
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("Struct::operator==(const Struct&) called on invalid Struct");
	}
	return types_ == other.types_;
}

bool Struct::isValid() const
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



Pointer::Pointer(TypeVariant pointerType) : pointerType_(new TypeVariant)
{
	*pointerType_ = pointerType;
	if(getValidationLevel() >= ValidationLevel::basic)
	{
		if(!isValid())
			throw std::invalid_argument("Pointer::Pointer(TypeVariant) Invalid Pointer parameters");
	}
}

TypeVariant Pointer::pointerType() const 
{
	if(pointerType_ == nullptr)
		throw std::runtime_error("Pointer::pointerType() called on null pointerType_");
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("Pointer::pointerType() called on invalid Pointer");
	}
	return *pointerType_; 
}

Pointer::Pointer(const Pointer& other) : pointerType_(new TypeVariant()) 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid Pointer::Pointer(const Pointer&) parameters");
	}
	*pointerType_ = other.pointerType();
}

Pointer::Pointer(Pointer&& other) 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid Pointer::Pointer(Pointer&&) parameters");
	}
	pointerType_ = std::move(other.pointerType_);
}

Pointer& Pointer::operator=(const Pointer& other)
{
	if (this == &other)
		return *this;
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid Pointer::operator=(const Pointer&) parameters");
	}
	if(pointerType_ == nullptr)
		pointerType_ = std::make_unique<TypeVariant>();
	*pointerType_ = other.pointerType();
	
	return *this;
}

Pointer& Pointer::operator=(Pointer&& other)
{
	if (this == &other)
		return *this;
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid Pointer::operator=(Pointer&&) parameters");
	}
	pointerType_ = std::move(other.pointerType_);
	return *this;
}

bool Pointer::isValid() const
{
	if(pointerType_ == nullptr)
		return false;
	if(getValidationLevel() < ValidationLevel::basic)
		return true;
	return ::isValid(*pointerType_);
}

bool Pointer::operator==(const Pointer& other) const
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Pointer::operator==(const Pointer&) invalid other Pointer");
	}
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("Pointer::operator==(const Pointer&) called on invalid Pointer");
	}
	return pointerType() == other.pointerType();
}



Array::Array(TypeVariant elementType, size_t count) : elementType_(new TypeVariant), count_(count) 
{
	*elementType_ = elementType;
	if(getValidationLevel() >= ValidationLevel::basic)
	{
		if(!isValid())
			throw std::invalid_argument("Array::Array(TypeVariant, size_t) Invalid Array parameters");
	}
}


TypeVariant Array::elementType() const 
{
	if(elementType_ == nullptr)
		throw std::runtime_error("Array::elementType() called on null elementType_");
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("Array::elementType() called on invalid Array");
	}
	return *elementType_;
}

size_t Array::count() const 
{
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("Array::count() called on invalid Array");
	}
	return count_; 
}

size_t Array::size() const 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!isValid())
			throw std::runtime_error("Array::size() called on invalid Array");
	}
	 return sizeOfTypeVariant(elementType()) * count_;
}

Array::Array(Array&& other) : count_(other.count_) 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid Array::Array(Array&&) parameters");
	}
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("Array::Array(Array&&) called on invalid Array");
	}
	elementType_ = std::move(other.elementType_);
	other.count_ = 0;
}

Array::Array(const Array& other) : elementType_(new TypeVariant), count_(other.count_) 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid Array::Array(const Array&) parameters");
	}
	*elementType_ = other.elementType();
}

Array& Array::operator=(Array&& other)
{
	if (this == &other)
		return *this;
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid Array::operator=(Array&&) parameters");
	}
	elementType_ = std::move(other.elementType_);
	count_ = other.count_;
	other.count_ = 0;
	other.elementType_ = nullptr;
	return *this;
}

Array& Array::operator=(const Array& other)
{
	if (this == &other)
		return *this;
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid Array::operator=(const Array&) parameters");
	}
	if(elementType_ == nullptr)
		elementType_ = std::make_unique<TypeVariant>();
	*elementType_ = other.elementType();
	count_ = other.count_;
	return *this;
}

bool Array::isValid() const
{
	if(elementType_ == nullptr)
		return false;
	if(count_ == 0)
		return false;
	if(getValidationLevel() < ValidationLevel::basic)
		return true;
	return ::isValid(elementType_);
}

bool Array::operator==(const Array& other) const
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!isValid() || !other.isValid())
			throw std::runtime_error("Array::operator== called on invalid Array");
	}
	return elementType() == other.elementType() && count_ == other.count_;
}

bool Array::isTypeCompatible(const Array& other) const
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!isValid() || !other.isValid())
			throw std::runtime_error("Array::isTypeCompatible called on invalid Array");
	}
	return elementType() == other.elementType();
}



Function::Function(TypeVariant returnType, const std::vector<TypeVariant>& argumentsTypes) :
	returnType_(new TypeVariant), argumentsTypes_(argumentsTypes) 
{
	*returnType_ = returnType;
	if(getValidationLevel() >= ValidationLevel::basic)
	{
		if(!isValid())
			throw std::invalid_argument("Function::Function(TypeVariant, const std::vector<TypeVariant>&) Invalid Function parameters");
	}
}

Function::Function(Function&& other) 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid Function::Function(Function&&) parameters");
	}
	returnType_ = std::move(other.returnType_);
	argumentsTypes_ = std::move(other.argumentsTypes_);
}

Function::Function(const Function& other) 
{
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid Function::Function(const Function&) parameters");
	}
	returnType_ = std::make_unique<TypeVariant>();
	*returnType_ = other.returnType();
	argumentsTypes_ = other.argumentsTypes_;
}

Function& Function::operator=(const Function& other)
{
	if (this == &other)
		return *this;

	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid Function::operator=(const Function&) parameters");
	}

	if(returnType_ == nullptr)
		returnType_ = std::make_unique<TypeVariant>();

	*returnType_ = other.returnType();
	argumentsTypes_ = other.argumentsTypes_;
	return *this;
}

Function& Function::operator=(Function&& other)
{
	if (this == &other)
		return *this;

	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!other.isValid())
			throw std::invalid_argument("Invalid Function::operator=(Function&&) parameters");
	}

	returnType_ = std::move(other.returnType_);
	argumentsTypes_ = std::move(other.argumentsTypes_);
	return *this;
}

bool Function::isValid() const
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

TypeVariant Function::returnType() const 
{
	if(returnType_ == nullptr)
		throw std::runtime_error("Function::returnType() called on invalid Function");
	if(getValidationLevel() >= ValidationLevel::light)
	{
		if(!isValid())
			throw std::runtime_error("Function::returnType() called on invalid Function");
	}
	 return *returnType_; 
}

const std::vector<TypeVariant>& Function::argumentsTypes() const 
{
	if(getValidationLevel() >= ValidationLevel::full)
	{
		if(!isValid())
			throw std::runtime_error("std::vector<TypeVariant>& Function::argumentsTypes() called on invalid Function");
	}
	 return argumentsTypes_; 
}

size_t Function::size() const 
{
	return sizeof(std::vector<Instruction>);
}

bool Function::operator==(const Function& other) const
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
	else if(isStruct(var))
		{
			if(std::get<const Struct*>(var) == nullptr)
				return false;
			return std::get<const Struct*>(var)->isValid();
		}
	else if(isPointer(var))
		return std::get<Pointer>(var).isValid();
	else if(isFunction(var))
	{
		return std::get<Function>(var).isValid();
	}
	else if(isArray(var))
	{
		return std::get<Array>(var).isValid();
	}
	else if(isStackLink(var))
	{
		return std::get<StackLink>(var).isValid();
	}
	return false;
}

bool isBaseType(const TypeVariant& var) 
{
	return std::holds_alternative<const BaseType*>(var);
}

bool isStruct(const TypeVariant& var) 
{
	return std::holds_alternative<const Struct*>(var);
}

bool isPointer(const TypeVariant& var) 
{
	return std::holds_alternative<Pointer>(var);
}

bool isFunction(const TypeVariant& var) 
{
	return std::holds_alternative<Function>(var);
}

bool isArray(const TypeVariant& var) 
{
	return std::holds_alternative<Array>(var);
}

bool isStackLink(const TypeVariant& var) 
{
	return std::holds_alternative<StackLink>(var);
}

size_t sizeOfTypeVariant(const TypeVariant& var)
{
	if (isBaseType(var))
		return std::get<const BaseType*>(var)->size();
	else if (isStruct(var))
		return std::get<const Struct*>(var)->size();
	else if (isFunction(var))
		return std::get<Function>(var).size();
	else if (isPointer(var))
		return std::get<Pointer>(var).size();
	else if (isArray(var))
		return std::get<Array>(var).size();
	else if (isStackLink(var))
		return std::get<StackLink>(var).size();
	return 0;
}

bool isBaseType(const std::unique_ptr<TypeVariant>& var) 
{
	if(var == nullptr)
		return false;
	return isBaseType(*var);
}

bool isStruct(const std::unique_ptr<TypeVariant>& var) 
{
	if(var == nullptr)
		return false;
	return isStruct(*var);
}

bool isPointer(const std::unique_ptr<TypeVariant>& var) 
{
	if(var == nullptr)
		return false;
	return isPointer(*var);
}

bool isFunction(const std::unique_ptr<TypeVariant>& var) 
{
	if(var == nullptr)
		return false;
	return isFunction(*var);
}

bool isArray(const std::unique_ptr<TypeVariant>& var) 
{
	if(var == nullptr)
		return false;
	return isArray(*var);
}

bool isStackLink(const std::unique_ptr<TypeVariant>& var) 
{
	if(var == nullptr)
		return false;
	return isStackLink(*var);
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
	else if (isStruct(a))
		return std::get<const Struct*>(a) == std::get<const Struct*>(b);
	else if (isFunction(a))
		return std::get<Function>(a) == std::get<Function>(b);
	else if (isPointer(a))
		return std::get<Pointer>(a) == std::get<Pointer>(b);
	else if (isArray(a))
		return std::get<Array>(a) == std::get<Array>(b);
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
	else if (isStruct(a))
		return std::get<const Struct*>(a) == std::get<const Struct*>(b);
	else if (isFunction(a))
		return std::get<Function>(a) == std::get<Function>(b);
	else if (isPointer(a))
		return std::get<Pointer>(a) == std::get<Pointer>(b);
	else if (isArray(a))
		return std::get<Array>(a).isTypeCompatible(std::get<Array>(b));
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
