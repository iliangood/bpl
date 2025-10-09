#include <gtest/gtest.h>
#include "variables/stack.h"

// Тестовый фикстур для упрощения работы с классом Stack
class StackTest : public ::testing::Test 
{
protected:
    void SetUp() override {
        // Создаем стек с емкостью 100 байт
        stack_ = Stack::make(100, false);
        ASSERT_TRUE(stack_.has_value()) << "Failed to create stack";
    }

    std::optional<Stack> stack_;
};

// Тест на создание стека
TEST_F(StackTest, CreateStack) 
{
    EXPECT_EQ(stack_->capacity(), 100u) << "Stack capacity should be 100";
    EXPECT_EQ(stack_->size(), 0u) << "Stack should be empty initially";
    EXPECT_TRUE(stack_->empty()) << "Stack should be empty";
}

// Тест на добавление элемента
TEST_F(StackTest, PushElement) 
{
    ElementInfo info("test_element", 10);
    auto ptr = stack_->push(info);
    ASSERT_TRUE(ptr.has_value()) << "Push should succeed";
    EXPECT_EQ(stack_->size(), 10u) << "Stack size should be 10 after push";
    EXPECT_FALSE(stack_->empty()) << "Stack should not be empty after push";
    EXPECT_EQ(stack_->element(0).name(), "test_element") << "Element name should match";
    EXPECT_EQ(stack_->element(0).size(), 10u) << "Element size should match";
}

// Тест на удаление элемента
TEST_F(StackTest, PopElement)
{
    ElementInfo info("test_element", 10);
    stack_->push(info);
    stack_->pop();
    EXPECT_EQ(stack_->size(), 0u) << "Stack should be empty after pop";
    EXPECT_TRUE(stack_->empty()) << "Stack should be empty after pop";
}

// Тест на доступ к элементам
TEST_F(StackTest, AccessElements) 
{
    ElementInfo info1("first", 5);
    ElementInfo info2("second", 10);
    stack_->push(info1);
    stack_->push(info2);
    EXPECT_EQ(stack_->at(0), stack_->operator[](0)) << "at(0) and operator[](0) should match";
    EXPECT_EQ(stack_->atFromEnd(0), stack_->operator[](1)) << "atFromEnd(0) should match last element";
    EXPECT_EQ(stack_->element(0).name(), "first") << "First element name should be 'first'";
    EXPECT_EQ(stack_->elementFromEnd(0).name(), "second") << "Last element name should be 'second'";
}

// Тест на изменение размера
TEST_F(StackTest, ResizeStack) 
{
    ElementInfo info("large_element", 150);
    auto ptr = stack_->push(info);
    ASSERT_TRUE(ptr.has_value()) << "Push should succeed after resize";
    EXPECT_GE(stack_->capacity(), 150u) << "Capacity should increase to accommodate large element";
    EXPECT_EQ(stack_->size(), 150u) << "Stack size should be 150 after push";
}

// Тест на очистку стека
TEST_F(StackTest, ClearStack) 
{
    ElementInfo info("test_element", 10);
    stack_->push(info);
    stack_->clear();
    EXPECT_EQ(stack_->size(), 0u) << "Stack should be empty after clear";
    EXPECT_TRUE(stack_->empty()) << "Stack should be empty after clear";
}

// Тест на уровни стека
TEST_F(StackTest, StackLevels) 
{
    ElementInfo info("test_element", 10);
    stack_->push(info);
    stack_->newLevel();
    stack_->push(info);
    EXPECT_EQ(stack_->currentLevel(), 2u) << "Should have 2 levels";
    stack_->deleteLevel();
    EXPECT_EQ(stack_->size(), 10u) << "Stack size should be 10 after deleting level";
    EXPECT_EQ(stack_->currentLevel(), 1u) << "Should have 1 level after deletion";
}