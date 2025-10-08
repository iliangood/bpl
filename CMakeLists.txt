cmake_minimum_required(VERSION 3.28.3)
project(ngl)

add_compile_options(-std=c++17 -Wall -Wextra -Wpedantic)

include_directories(include)

add_library(stack src/stack.cpp include/stack.h)

add_library(interpreter src/interpreter/interpreter.cpp include/interpreter/interpreter.h)

add_executable(ngl src/ngl.cpp)

enable_testing()
add_subdirectory(tests)

add_custom_target(tests
    COMMAND ctest --verbose
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests with verbose output"
)