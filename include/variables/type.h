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


class Function
{
	std::unique_ptr<TypeVariant> returnType_;
	std::vector<TypeVariant> argumentsTypes_;
public:
	Function(TypeVariant returnType, const std::vector<TypeVariant>& argumentsTypes);
	
	bool isValid() const;

	Function(Function&&);
	Function(const Function&);
	Function& operator=(Function&&);
	Function& operator=(const Function&);

	bool operator==(const Function& other) const;
	bool operator!=(const Function& other) const { return !(*this == other); }

	size_t size() const;
	TypeVariant returnType() const;
	const std::vector<TypeVariant>& argumentsTypes() const;
};


class Pointer
{
	std::unique_ptr<TypeVariant> pointerType_;
	bool onStack_;
public:
	Pointer(TypeVariant pointerType, bool onStack);

	bool isValid() const;

	bool onStack();

	Pointer(const Pointer& other);
	Pointer(Pointer&& other);
	Pointer& operator=(const Pointer& other);
	Pointer& operator=(Pointer&& other);

	bool operator==(const Pointer& other) const;
	bool operator!=(const Pointer& other) const { return !(*this == other); }

	TypeVariant pointerType() const;
	size_t size() const { return sizeof(void*); }
};

class Array
{
	std::unique_ptr<TypeVariant> elementType_;
	size_t count_;
public:
	Array(TypeVariant elementType, size_t count);
	Array(Array&&);
	Array(const Array&);
	Array& operator=(Array&&);
	Array& operator=(const Array&);

	bool isValid() const;

	bool operator==(const Array& other) const;
	bool operator!=(const Array& other) const { return !(*this == other); }
	bool isTypeCompatible(const Array& other) const;
	bool TypeCmp(const Array& other) const { return isTypeCompatible(other); }
	bool typecheck(const Array& other) const { return TypeCmp(other); }

	TypeVariant elementType() const;
	size_t count() const;
	size_t size() const;
};

class Struct
{
	std::string name_;
	std::vector<TypeVariant> types_;
	size_t totalSize_;
public:
	Struct(std::string name, const std::vector<TypeVariant>& types);
	Struct(Struct&&) = default;
	Struct(const Struct&) = default;
	Struct& operator=(Struct&&) = default;
	Struct& operator=(const Struct&) = default;

	bool isValid() const;

	bool operator==(const Struct& other) const;
	bool operator!=(const Struct& other) const { return !(*this == other); }

	std::string name() const;
	size_t size() const;

	const std::vector<TypeVariant>& baseTypes() const;
};

class TypeVariant : public std::variant<const BaseType*, const Struct*, Function, Pointer, Array>
{
public:
    using std::variant<const BaseType*, const Struct*, Function, Pointer, Array>::variant;

    using std::variant<const BaseType*, const Struct*, Function, Pointer, Array>::index;
    using std::variant<const BaseType*, const Struct*, Function, Pointer, Array>::operator=;
};

bool isValid(const TypeVariant& var);
bool isValid(const std::unique_ptr<TypeVariant>& var);

bool isBaseType(const TypeVariant& var);
bool isStruct(const TypeVariant& var);
bool isPointer(const TypeVariant& var);
bool isFunction(const TypeVariant& var);
bool isArray(const TypeVariant& var);
size_t sizeOfTypeVariant(const TypeVariant& var);

bool isBaseType(const std::unique_ptr<TypeVariant>& var);
bool isStruct(const std::unique_ptr<TypeVariant>& var);
bool isPointer(const std::unique_ptr<TypeVariant>& var);
bool isFunction(const std::unique_ptr<TypeVariant>& var);
bool isArray(const std::unique_ptr<TypeVariant>& var);
size_t sizeOfTypeVariant(const std::unique_ptr<TypeVariant>& var);

bool operator==(const TypeVariant& a, const TypeVariant& b);
bool operator!=(const TypeVariant& a, const TypeVariant& b);

bool isTypeCompatible(const TypeVariant& a, const TypeVariant& b);
bool isTypeCompatible(const TypeVariant* a, const TypeVariant* b);
bool isTypeCompatible(const std::unique_ptr<TypeVariant>& a, const std::unique_ptr<TypeVariant>& b);


#endif