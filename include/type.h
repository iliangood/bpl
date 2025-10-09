#if !defined TYPE_H
#define TYPE_H

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <cstddef>

class BaseType
{
	std::string name_;
	size_t size_;
public:
	BaseType(std::string name, size_t size) : name_(name), size_(size) {}
	size_t size() const { return size_; }
	std::string name() const { return name_; }
};

class Pointer;
class Type;

typedef std::variant<const BaseType*, const Type*, const Pointer*> TypeVariant;

class Pointer
{
	TypeVariant* pointsTo_;
	void* ptr_;
public:
	Pointer(TypeVariant* pointsTo) : pointsTo_(pointsTo), ptr_(nullptr) {}
	TypeVariant* pointsTo() const { return pointsTo_; }
	size_t size() const { return sizeof(void*); }
	void* ptr() const { return ptr_; }
	void setPtr(void* ptr) { ptr_ = ptr; }
};

class Type
{
	std::string name_;
	std::vector<TypeVariant> types_;
	size_t totalSize_;

public:
	Type(std::string name, const std::vector<TypeVariant>& types);

	std::string name() const { return name_; }
	size_t size() const { return totalSize_; }

	const std::vector<TypeVariant>& baseTypes() const { return types_; }
};


class Array
{
	Type* elementType_;
	size_t count_;
public:
	Array(Type* elementType, size_t count) : elementType_(elementType), count_(count) {}
	Type* elementType() const { return elementType_; }
	size_t count() const { return count_; }
	size_t size() const { return elementType_->size() * count_; }
};
#endif