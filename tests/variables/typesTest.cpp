#include <gtest/gtest.h>
#include "variables/type.h"

// Test BaseType class
TEST(BaseTypeTest, ConstructorAndGetters) {
    BaseType bt("int", 4);
    EXPECT_EQ(bt.name(), "int");
    EXPECT_EQ(bt.size(), 4);
}

// Test Pointer class
TEST(PointerTest, ConstructorAndGetters) {
    BaseType bt("int", 4);
    TypeVariant tv = &bt;  // TypeVariant holds const BaseType*
    Pointer ptr(&tv);
    EXPECT_EQ(ptr.pointsTo(), &tv);
    EXPECT_EQ(ptr.size(), sizeof(void*));
    EXPECT_EQ(ptr.ptr(), nullptr);

    void* dummy = reinterpret_cast<void*>(0x1234);
    ptr.setPtr(dummy);
    EXPECT_EQ(ptr.ptr(), dummy);
}

// Test Type class with BaseTypes
TEST(TypeTest, ConstructorWithBaseTypes) {
    BaseType intType("int", 4);
    BaseType floatType("float", 4);
    std::vector<TypeVariant> types = {&intType, &floatType};
    Type composite("Pair", types);

    EXPECT_EQ(composite.name(), "Pair");
    EXPECT_EQ(composite.size(), 8);
    const auto& baseTypes = composite.baseTypes();
    EXPECT_EQ(baseTypes.size(), 2);
    EXPECT_TRUE(std::holds_alternative<const BaseType*>(baseTypes[0]));
    EXPECT_EQ(std::get<const BaseType*>(baseTypes[0])->name(), "int");
    EXPECT_TRUE(std::holds_alternative<const BaseType*>(baseTypes[1]));
    EXPECT_EQ(std::get<const BaseType*>(baseTypes[1])->name(), "float");
}

// Test Type class with nested Types
TEST(TypeTest, ConstructorWithNestedTypes) {
    BaseType intType("int", 4);
    std::vector<TypeVariant> innerTypes = {&intType};
    Type inner("Inner", innerTypes);

    std::vector<TypeVariant> outerTypes = {&inner};
    Type outer("Outer", outerTypes);

    EXPECT_EQ(outer.name(), "Outer");
    EXPECT_EQ(outer.size(), 4);
    const auto& baseTypes = outer.baseTypes();
    EXPECT_EQ(baseTypes.size(), 1);
    EXPECT_TRUE(std::holds_alternative<const Type*>(baseTypes[0]));
    EXPECT_EQ(std::get<const Type*>(baseTypes[0])->name(), "Inner");
}

// Test Type class with Pointers
TEST(TypeTest, ConstructorWithPointers) {
    BaseType intType("int", 4);
    TypeVariant tv = &intType;
    Pointer ptr(&tv);

    std::vector<TypeVariant> types = {&ptr};
    Type composite("PtrHolder", types);

    EXPECT_EQ(composite.name(), "PtrHolder");
    EXPECT_EQ(composite.size(), sizeof(void*));
    const auto& baseTypes = composite.baseTypes();
    EXPECT_EQ(baseTypes.size(), 1);
    EXPECT_TRUE(std::holds_alternative<const Pointer*>(baseTypes[0]));
    EXPECT_EQ(std::get<const Pointer*>(baseTypes[0])->pointsTo(), &tv);
}

// Test Type class with mixed variants
TEST(TypeTest, ConstructorWithMixed) {
    BaseType intType("int", 4);
    BaseType floatType("float", 4);

    TypeVariant intTv = &intType;
    Pointer intPtr(&intTv);

    std::vector<TypeVariant> innerTypes = {&floatType};
    Type inner("Inner", innerTypes);

    std::vector<TypeVariant> types = {&intType, &intPtr, &inner};
    Type composite("Mixed", types);

    EXPECT_EQ(composite.name(), "Mixed");
    EXPECT_EQ(composite.size(), 4 + sizeof(void*) + 4);
}

// Test Array class
TEST(ArrayTest, ConstructorAndGetters) {
    BaseType intType("int", 4);
    std::vector<TypeVariant> types = {&intType};
    Type arrayElem("Elem", types);

    Array arr(&arrayElem, 5);

    EXPECT_EQ(arr.elementType(), &arrayElem);
    EXPECT_EQ(arr.count(), 5);
    EXPECT_EQ(arr.size(), 4 * 5);
}

// Test Array with composite Type
TEST(ArrayTest, SizeWithComposite) {
    BaseType intType("int", 4);
    BaseType floatType("float", 4);
    std::vector<TypeVariant> types = {&intType, &floatType};
    Type pair("Pair", types);

    Array arr(&pair, 3);

    EXPECT_EQ(arr.size(), 8 * 3);
}