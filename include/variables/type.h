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
	BaseType() : size_(0) {}
	BaseType(std::string name, size_t size);
	BaseType(BaseType&&) = default;
	BaseType(const BaseType&) = default;
	BaseType& operator=(BaseType&&) = default;
	BaseType& operator=(const BaseType&) = default;

	bool isValid() const;

	size_t size() const;
	std::string name() const;

	size_t elementCount() const { return 1; }
	std::vector<size_t> elementSubIndexes() const { return {0}; }
};

class TypeVariant;


class FunctionType
{
	std::vector<TypeVariant> argumentsTypes_;
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
	const TypeVariant& returnType() const;
	TypeVariant& returnType();
	const std::vector<TypeVariant>& argumentsTypes() const;
	std::vector<TypeVariant>& argumentsTypes();

	size_t elemetCount() const { return 1; }
	std::vector<size_t> elementSubIndexes() const { return {0}; }
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

	size_t elementCount() const { return 1; }
	std::vector<size_t> elementSubIndexes() const { return {0}; }
};

class LinkType
{
	std::unique_ptr<TypeVariant> elementType_;
public:
	LinkType(TypeVariant elementType);
	LinkType();
	LinkType(const LinkType& other);
	LinkType(LinkType&& other);

	LinkType operator=(const LinkType& other);
	LinkType operator=(LinkType&& other);
	
	bool isValid() const;

	size_t size() const;
	bool isGlobal() const { return elementType_ != nullptr; }
	std::optional<TypeVariant> pointsTo() const;


	bool operator==(const LinkType&) const { return true; }
	bool operator!=(const LinkType& other) const { return !(*this == other); }

	size_t elementCount() const { return 1; }
	std::vector<size_t> elementSubIndexes() const { return {0}; }
};

class ArrayType
{
	std::unique_ptr<TypeVariant> elementType_;
	size_t count_;
public:
	ArrayType(TypeVariant elementType, size_t count);
	ArrayType(ArrayType&& other);
	ArrayType(const ArrayType& other);
	ArrayType& operator=(ArrayType&& other);
	ArrayType& operator=(const ArrayType& other);

	bool isValid() const;

	bool operator==(const ArrayType& other) const;
	bool operator!=(const ArrayType& other) const { return !(*this == other); }

	TypeVariant elementType() const;
	size_t count() const;
	size_t size() const;

	size_t elementCount() const;
	std::vector<size_t> elementSubIndexes() const;
};



class StructType
{
	std::vector<TypeVariant> types_;
	std::vector<std::string> fieldNames_;
	size_t totalSize_;
public:
	StructType(const std::vector<TypeVariant>& types, const std::vector<std::string>& fieldNames);
	StructType(StructType&&) = default;
	StructType(const StructType&) = default;
	StructType& operator=(StructType&&) = default;
	StructType& operator=(const StructType&) = default;

	bool isValid() const;

	bool operator==(const StructType& other) const;
	bool operator!=(const StructType& other) const { return !(*this == other); }

	size_t size() const;

	const std::vector<TypeVariant>& types() const;
	TypeVariant type(size_t index) const;
	const std::vector<std::string>& fieldNames() const { return fieldNames_; }

	size_t elementCount() const { return types_.size() + 1; }
	std::vector<size_t> elementSubIndexes() const;
	size_t elementSubIndex(size_t index) const;
	size_t offsetBySize(size_t index) const;
	std::vector<size_t> offsetsBySize() const; // Returns offsets of struct and its fields
};



class TypeVariant : public std::variant<const BaseType*, const StructType*, FunctionType, PointerType, ArrayType, LinkType>
{
private:
	using Base = std::variant<const BaseType*, const StructType*, FunctionType, PointerType, ArrayType, LinkType>;

public:
	TypeVariant() = default;
	TypeVariant(const TypeVariant&) = default;
	TypeVariant(TypeVariant&&) noexcept = default;
	TypeVariant& operator=(const TypeVariant&) = default;
	TypeVariant& operator=(TypeVariant&&) noexcept = default;


TypeVariant(const BaseType* ptr)
	: Base(std::in_place_type<const BaseType*>, ptr)
{}

TypeVariant(const StructType* ptr)
	: Base(std::in_place_type<const StructType*>, ptr)
{}

template<typename T,
		 typename = std::enable_if_t<
			!std::is_same_v<std::decay_t<T>, TypeVariant> &&
			!std::is_same_v<std::decay_t<T>, BaseType*> &&
			!std::is_same_v<std::decay_t<T>, const BaseType*> &&
			!std::is_same_v<std::decay_t<T>, const StructType*> &&
			!std::is_same_v<std::decay_t<T>, StructType*> &&
			(std::is_same_v<std::decay_t<T>, FunctionType> ||
			std::is_same_v<std::decay_t<T>, PointerType> ||
			std::is_same_v<std::decay_t<T>, ArrayType> ||
			std::is_same_v<std::decay_t<T>, LinkType>)
		 >>
TypeVariant(T&& value)
	: Base(std::in_place_type<std::decay_t<T>>, std::forward<T>(value))
{}
	template<typename T,
		 typename = std::enable_if_t<
			 !std::is_same_v<std::decay_t<T>, TypeVariant> &&
			 (std::is_same_v<std::decay_t<T>, BaseType*> || 
			  std::is_same_v<std::decay_t<T>, const BaseType*> ||
			  std::is_same_v<std::decay_t<T>, const StructType*> ||
			  std::is_same_v<std::decay_t<T>, FunctionType> ||
			  std::is_same_v<std::decay_t<T>, PointerType> ||
			  std::is_same_v<std::decay_t<T>, ArrayType> ||
			  std::is_same_v<std::decay_t<T>, LinkType>)
		 >>
TypeVariant(T& value)
	: Base(std::in_place_type<std::decay_t<T>>, value)
{}


	bool isValid() const;
	bool isBaseType() const;
	bool isStructType() const;
	bool isPointerType() const;
	bool isFunctionType() const;
	bool isArrayType() const;
	bool isLinkType() const;
	size_t size() const;
	size_t elementCount() const;
	
	bool operator!=(const TypeVariant& other) const;
	bool operator==(const TypeVariant& other) const;

	template<typename T>
	T& get() 
	{
		return std::get<T>(*this);
	}
	
	template<typename T>
	const T& get() const 
	{
		return std::get<T>(*this);
	}
};


#endif