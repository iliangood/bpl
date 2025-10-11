#include "variables/type.h"

BaseType::BaseType(std::string name, size_t size) : name_(std::move(name)), size_(size) {}

size_t BaseType::size() const { return size_; }

std::string BaseType::name() const { return name_; }



Struct::Struct(std::string name, const std::vector<TypeVariant>& types) : name_(std::move(name)), types_(types), totalSize_(0)
{
	for (size_t i = 0; i < types_.size(); ++i)
	{
		totalSize_ += sizeOfTypeVariant(types_[i]);
	}
}

std::string Struct::name() const { return name_; }

size_t Struct::size() const { return totalSize_; }

const std::vector<TypeVariant>& Struct::baseTypes() const { return types_; }

bool Struct::operator==(const Struct& other) const
{
	return types_ == other.types_;
}



Pointer::Pointer(TypeVariant pointerType) : pointerType_(new TypeVariant)
{
	*pointerType_ = pointerType;
}

Pointer::~Pointer()
{
	pointerType_ = nullptr;
}

TypeVariant Pointer::pointerType() const { return *pointerType_; }

Pointer::Pointer(const Pointer& other) : pointerType_(new TypeVariant()) 
{
	*pointerType_ = *other.pointerType_;
}

Pointer::Pointer(Pointer&& other) : pointerType_(std::move(other.pointerType_)) {}

Pointer& Pointer::operator=(const Pointer& other)
{
	if (this != &other)
	{
		if(pointerType_ == nullptr)
			pointerType_ = std::make_unique<TypeVariant>();
		*pointerType_ = *other.pointerType_;
	}
	return *this;
}

Pointer& Pointer::operator=(Pointer&& other)
{
	if (this != &other)
	{
		pointerType_ = std::move(other.pointerType_);
	}
	return *this;
}

bool Pointer::operator==(const Pointer& other) const
{
	return *pointerType_ == *other.pointerType_;
}



Array::Array(TypeVariant elementType, size_t count) : elementType_(new TypeVariant), count_(count) 
{
	*elementType_ = elementType;
}

Array::~Array()
{
	elementType_ = nullptr;
}

TypeVariant Array::elementType() const { return *elementType_; }

size_t Array::count() const { return count_; }

size_t Array::size() const { return sizeOfTypeVariant(*elementType_) * count_; }

Array::Array(Array&& other) : elementType_(std::move(other.elementType_)), count_(other.count_) {}

Array::Array(const Array& other) : elementType_(new TypeVariant), count_(other.count_) 
{
	*elementType_ = *other.elementType_;
}

Array& Array::operator=(Array&& other)
{
	if (this != &other)
	{
		elementType_ = std::move(other.elementType_);
		count_ = other.count_;
		other.count_ = 0;
		other.elementType_ = nullptr;
	}
	return *this;
}

Array& Array::operator=(const Array& other)
{
	if (this != &other)
	{
		if(elementType_ == nullptr)
			elementType_ = std::make_unique<TypeVariant>();
		*elementType_ = *other.elementType_;
		count_ = other.count_;
	}
	return *this;
}

bool Array::operator==(const Array& other) const
{
	return *elementType_ == *other.elementType_ && count_ == other.count_;
}

bool Array::isTypeCompatible(const Array& other) const
{
	return *elementType_ == *other.elementType_;
}



Function::Function(TypeVariant returnType, const std::vector<TypeVariant>& argumentsTypes) :
	returnType_(new TypeVariant), argumentsTypes_(argumentsTypes) 
{
	*returnType_ = returnType;
}

Function::~Function()
{
	returnType_ = nullptr;
}

Function::Function(Function&& other) :
	returnType_(std::move(other.returnType_)), argumentsTypes_(std::move(other.argumentsTypes_)) {}

Function::Function(const Function& other) :
	returnType_(new TypeVariant), argumentsTypes_(other.argumentsTypes_) 
{
	*returnType_ = *other.returnType_;
}

Function& Function::operator=(const Function& other)
{
	if (this != &other)
	{
		if(returnType_ == nullptr)
			returnType_ = std::make_unique<TypeVariant>();
		*returnType_ = *other.returnType_;
		argumentsTypes_ = other.argumentsTypes_;
	}
	return *this;
}

Function& Function::operator=(Function&& other)
{
	if (this != &other)
	{
		returnType_ = std::move(other.returnType_);
		argumentsTypes_ = std::move(other.argumentsTypes_);
	}
	return *this;
}

TypeVariant Function::returnType() const { return *returnType_; }

const std::vector<TypeVariant>& Function::argumentsTypes() const { return argumentsTypes_; }

bool Function::operator==(const Function& other) const
{
	return *returnType_ == *other.returnType_ && argumentsTypes_ == other.argumentsTypes_;
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

size_t sizeOfTypeVariant(const std::unique_ptr<TypeVariant>& var)
{
	if(var == nullptr)
		return false;
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

bool isTypeCompatible(const TypeVariant* a, const TypeVariant* b)
{
	if (a == nullptr || b == nullptr)
		return false;
	return isTypeCompatible(*a, *b);
}
