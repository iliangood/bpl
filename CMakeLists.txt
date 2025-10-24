cmake_minimum_required(VERSION 3.28.3)
project(ngl)

add_compile_options(-std=c++17 -Wall -Wextra -Wpedantic)

include_directories(include)

add_library(utils STATIC src/utils.cpp include/utils.h)

add_library(stack STATIC src/variables/stack.cpp include/variables/stack.h)

add_library(interpreter STATIC src/interpreter/processor.cpp include/interpreter/processor.h)

add_library(variables STATIC src/variables/type.cpp include/variables/type.h)

target_link_libraries(interpreter variables stack utils)

add_executable(ngl src/ngl.cpp)

enable_testing()
add_subdirectory(tests)

add_custom_target(tests
    COMMAND ctest --verbose
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests with verbose output"
)