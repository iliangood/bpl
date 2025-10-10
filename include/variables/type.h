#if !defined TYPE_H
#define TYPE_H

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <cstddef>
#include <memory>

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
	size_t size() const;
	std::string name() const;
};

class TypeVariant;


class Function
{
	std::shared_ptr<TypeVariant> returnType_;
	std::vector<TypeVariant> argumentsTypes_;
public:
	Function(TypeVariant returnType, const std::vector<TypeVariant>& argumentsTypes);

	Function(Function&&);
	Function(const Function&);
	Function& operator=(Function&&);
	Function& operator=(const Function&);

	bool operator==(const Function& other) const;
	bool operator!=(const Function& other) const { return !(*this == other); }

	size_t size() const { return sizeof(void*); }
	TypeVariant returnType() const;
	const std::vector<TypeVariant>& argumentsTypes() const;
};


class Pointer
{
	std::shared_ptr<TypeVariant> pointerType_;
public:
	Pointer(TypeVariant pointerType);

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
	std::shared_ptr<TypeVariant> elementType_;
	size_t count_;
public:
	Array(TypeVariant elementType, size_t count);
	Array(Array&&);
	Array(const Array&);
	Array& operator=(Array&&);
	Array& operator=(const Array&);

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

	std::string name() const;
	size_t size() const;

	const std::vector<TypeVariant>& baseTypes() const;
};

class TypeVariant : public std::variant<const BaseType*, const Struct*, const Function*, Pointer, Array>
{
public:
    // Перенаправляем конструкторы variant
    using std::variant<const BaseType*, const Struct*, const Function*, Pointer, Array>::variant;

    // Перенаправляем другие ключевые методы (holds_alternative, get и т.д.)
    using std::variant<const BaseType*, const Struct*, const Function*, Pointer, Array>::index;
    using std::variant<const BaseType*, const Struct*, const Function*, Pointer, Array>::operator=;
    // Добавьте другие, если нужно (visit, emplace и т.д.)
};


bool isBaseType(const TypeVariant& var);
bool isStruct(const TypeVariant& var);
bool isPointer(const TypeVariant& var);
bool isFunction(const TypeVariant& var);
bool isArray(const TypeVariant& var);
size_t sizeOfTypeVariant(const TypeVariant& var);

bool isBaseType(std::shared_ptr<TypeVariant> var);
bool isStruct(std::shared_ptr<TypeVariant> var);
bool isPointer(std::shared_ptr<TypeVariant> var);
bool isFunction(std::shared_ptr<TypeVariant> var);
bool isArray(std::shared_ptr<TypeVariant> var);
size_t sizeOfTypeVariant(std::shared_ptr<TypeVariant> var);

bool operator==(const TypeVariant& a, const TypeVariant& b);
bool operator!=(const TypeVariant& a, const TypeVariant& b);


#endif