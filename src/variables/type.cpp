#include "variables/type.h"

BaseType::BaseType(std::string name, size_t size) : name_(std::move(name)), size_(size) {}

size_t BaseType::size() const { return size_; }

std::string BaseType::name() const { return name_; }



Type::Type(std::string name, const std::vector<TypeVariant>& types) : name_(std::move(name)), types_(types), totalSize_(0)
{
	for (size_t i = 0; i < types_.size(); ++i)
	{
		if (std::holds_alternative<const BaseType*>(types_[i]))
			totalSize_ += std::get<const BaseType*>(types_[i])->size();
		else if (std::holds_alternative<const Type*>(types_[i]))
			totalSize_ += std::get<const Type*>(types_[i])->size();
		else if (std::holds_alternative<const Pointer*>(types_[i]))
			totalSize_ += std::get<const Pointer*>(types_[i])->size();
	}
}

std::string Type::name() const { return name_; }

size_t Type::size() const { return totalSize_; }

const std::vector<TypeVariant>& Type::baseTypes() const { return types_; }



Pointer::Pointer(TypeVariant* pointerType) : pointerType_(pointerType), ptr_(nullptr) {}

TypeVariant* Pointer::pointsTo() const { return pointerType_; }

size_t Pointer::size() const { return sizeof(void*); }

void* Pointer::ptr() const { return ptr_; }

void Pointer::setPtr(void* ptr) { ptr_ = ptr; }



Array::Array(Type* elementType, size_t count) : elementType_(elementType), count_(count) {}

Type* Array::elementType() const { return elementType_; }

size_t Array::count() const { return count_; }

size_t Array::size() const { return elementType_->size() * count_; }



Function::Function(std::string name, Type* returnType, const std::vector<TypeVariant>& argumentsTypes) :
	name_(std::move(name)), returnType_(returnType), argumentsTypes_(argumentsTypes), startsAddress_(0) {}

std::string Function::name() const { return name_; }

Type* Function::returnType() const { return returnType_; }

const std::vector<TypeVariant>& Function::argumentsTypes() const { return argumentsTypes_; }
