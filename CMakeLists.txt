cmake_minimum_required(VERSION 3.28.3)
project(bpl)

add_compile_options(-std=c++17 -Wall -Wextra -Wpedantic)

include_directories(include)

add_library(utils STATIC src/utils.cpp)

add_library(stack STATIC src/variables/stack.cpp)

add_library(types STATIC src/variables/type.cpp)

add_library(processor STATIC src/interpreter/processor.cpp)

add_library(variables STATIC src/variables/type.cpp)

target_link_libraries(processor PUBLIC variables stack utils)

target_link_libraries(stack PUBLIC types)

add_executable(bpl src/bpl.cpp)

target_link_libraries(bpl processor)

enable_testing()
add_subdirectory(tests)

add_custom_target(tests
    COMMAND ctest --verbose
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests with verbose output"
)