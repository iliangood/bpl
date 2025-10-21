#if !defined TYPE_H
#define TYPE_H

#include <string>
#include <cstring>
#include <vector>
#include <optional>
#include <variant>
#include <cstddef>
#include <memory>
#include <stdexcept>


#include "utils.h"

class BaseType
{
	std::string name_;
	size_t size_;
public:
	BaseType();
	BaseType(std::string name, size_t size);
	BaseType(BaseType&&) = default;
	BaseType(const BaseType&) = default;
	BaseType& operator=(BaseType&&) = default;
	BaseType& operator=(const BaseType&) = default;

	bool isValid() const;

	size_t size() const;
	std::string name() const;
};

class TypeVariant;


class FunctionType
{
	std::vector<TypeVariant> argumentsTypes_;
	std::vector<std::string> argumentNames_;
	std::unique_ptr<TypeVariant> returnType_;
public:
	FunctionType() : returnType_(nullptr) {}
	FunctionType(const std::vector<TypeVariant>& argumentsTypes, TypeVariant returnType);
	
	bool isValid() const;

	FunctionType(FunctionType&&);
	FunctionType(const FunctionType&);
	FunctionType& operator=(FunctionType&&);
	FunctionType& operator=(const FunctionType&);

	bool operator==(const FunctionType& other) const;
	bool operator!=(const FunctionType& other) const { return !(*this == other); }

	size_t size() const;
	bool hasReturnType() const { return returnType_ != nullptr; }
	TypeVariant returnType() const;
	const std::vector<TypeVariant>& argumentsTypes() const;
	const std::vector<std::string>& argumentNames() const;
};


class PointerType
{
	std::unique_ptr<TypeVariant> pointerType_;
public:
	PointerType(TypeVariant pointerType);

	bool isValid() const;

	PointerType(const PointerType& other);
	PointerType(PointerType&& other);
	PointerType& operator=(const PointerType& other);
	PointerType& operator=(PointerType&& other);

	bool operator==(const PointerType& other) const;
	bool operator!=(const PointerType& other) const { return !(*this == other); }

	TypeVariant pointerType() const;
	size_t size() const { return sizeof(void*); }
};

class StackLinkType
{
public:
	size_t size() const { return sizeof(size_t); }
	bool isValid() const { return true; }

	bool operator==(const StackLinkType&) const { return true; }
	bool operator!=(const StackLinkType& other) const { return !(*this == other); }
};

class ArrayType
{
	std::unique_ptr<TypeVariant> elementType_;
	size_t count_;
public:
	ArrayType(TypeVariant elementType, size_t count);
	ArrayType(ArrayType&&);
	ArrayType(const ArrayType&);
	ArrayType& operator=(ArrayType&&);
	ArrayType& operator=(const ArrayType&);

	bool isValid() const;

	bool operator==(const ArrayType& other) const;
	bool operator!=(const ArrayType& other) const { return !(*this == other); }

	TypeVariant elementType() const;
	size_t count() const;
	size_t size() const;
};



class StructType
{
	std::string name_;
	std::vector<TypeVariant> types_;
	std::vector<std::string> fieldNames_;
	size_t totalSize_;
public:
	StructType(std::string name, const std::vector<TypeVariant>& types, const std::vector<std::string>& fieldNames);
	StructType(StructType&&) = default;
	StructType(const StructType&) = default;
	StructType& operator=(StructType&&) = default;
	StructType& operator=(const StructType&) = default;

	bool isValid() const;

	bool operator==(const StructType& other) const;
	bool operator!=(const StructType& other) const { return !(*this == other); }

	std::string name() const;
	size_t size() const;

	const std::vector<TypeVariant>& types() const;
	const std::vector<std::string>& fieldNames() const { return fieldNames_; }
};



class TypeVariant : public std::variant<const BaseType*, const StructType*, FunctionType, PointerType, ArrayType, StackLinkType>
{
public:
    using std::variant<const BaseType*, const StructType*, FunctionType, PointerType, ArrayType, StackLinkType>::variant;

    using std::variant<const BaseType*, const StructType*, FunctionType, PointerType, ArrayType, StackLinkType>::index;
    using std::variant<const BaseType*, const StructType*, FunctionType, PointerType, ArrayType, StackLinkType>::operator=;
};

bool isValid(const TypeVariant& var);
bool isValid(const std::unique_ptr<TypeVariant>& var);

bool isBaseType(const TypeVariant& var);
bool isStructType(const TypeVariant& var);
bool isPointerType(const TypeVariant& var);
bool isFunctionType(const TypeVariant& var);
bool isArrayType(const TypeVariant& var);
bool isStackLinkType(const TypeVariant& var);
size_t sizeOfTypeVariant(const TypeVariant& var);

bool isBaseType(const std::unique_ptr<TypeVariant>& var);
bool isStructType(const std::unique_ptr<TypeVariant>& var);
bool isPointerType(const std::unique_ptr<TypeVariant>& var);
bool isFunctionType(const std::unique_ptr<TypeVariant>& var);
bool isArrayType(const std::unique_ptr<TypeVariant>& var);
bool isStackLinkType(const std::unique_ptr<TypeVariant>& var);
size_t sizeOfTypeVariant(const std::unique_ptr<TypeVariant>& var);

bool operator==(const TypeVariant& a, const TypeVariant& b);
bool operator!=(const TypeVariant& a, const TypeVariant& b);


#endif