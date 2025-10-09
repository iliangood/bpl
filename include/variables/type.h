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
	BaseType(std::string name, size_t size);
	size_t size() const;
	std::string name() const;
};

class Pointer;
class Type;

typedef std::variant<const BaseType*, const Type*, const Pointer*> TypeVariant;

class Pointer
{
	TypeVariant* pointerType_;
	void* ptr_;
public:
	Pointer(TypeVariant* pointerType);
	TypeVariant* pointsTo() const;
	size_t size() const;
	void* ptr() const;
	void setPtr(void* ptr);
};

class Type
{
	std::string name_;
	std::vector<TypeVariant> types_;
	size_t totalSize_;

public:
	Type(std::string name, const std::vector<TypeVariant>& types);

	std::string name() const;
	size_t size() const;

	const std::vector<TypeVariant>& baseTypes() const;
};


class Array
{
	Type* elementType_;
	size_t count_;
public:
	Array(Type* elementType, size_t count);
	Type* elementType() const;
	size_t count() const;
	size_t size() const;
};
#endif