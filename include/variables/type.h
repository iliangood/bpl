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

	size_t elementCount() const { return 1; }
	std::vector<size_t> elementSubIndexes() const { return {0}; }
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

class LinkType // ! Доделать этот класс
{
	std::unique_ptr<TypeVariant> elementType_;
public:
	LinkType(TypeVariant elementType);
	LinkType();
	LinkType(LinkType& other);
	LinkType(LinkType&& other);

	LinkType operator=(LinkType& other);
	LinkType operator=(LinkType&& other);
	
	bool isValid() const;

	size_t size() const;
	bool isGlobal() const { return elementType_ != nullptr; }


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
	
	template<typename T>
	void assign(const T& value) {
		static_cast<Base&>(*this) = value;
	}

	void copyFrom(const TypeVariant& other)
	{
		if(std::holds_alternative<const BaseType*>(other))
			assign(std::get<const BaseType*>(other));
		else if(std::holds_alternative<const StructType*>(other))
			assign(std::get<const StructType*>(other));
		else if(std::holds_alternative<FunctionType>(other))
			assign(std::get<FunctionType>(other));
		else if(std::holds_alternative<PointerType>(other))
			assign(std::get<PointerType>(other));
		else if(std::holds_alternative<ArrayType>(other))
			assign(std::get<ArrayType>(other));
		else if(std::holds_alternative<LinkType>(other))
			assign(std::get<LinkType>(other));
		else
			throw std::runtime_error("void TypeVariant::copyFrom(const TypeVariant&) called on unknow TypeVariant");
	}

public:
	TypeVariant() = default;
	
	template<typename T>
	TypeVariant(const T& value)
	{
		assign(value);
	}
	
	TypeVariant(const TypeVariant& other) : Base()
	{
		copyFrom(other);
	}
	
	TypeVariant& operator=(const TypeVariant& other)
	{
		if (this != &other) 
			copyFrom(other);
		return *this;
	}

	TypeVariant(TypeVariant&& other) noexcept = default;
	TypeVariant& operator=(TypeVariant&& other) noexcept = default;

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