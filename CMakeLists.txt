cmake_minimum_required(VERSION 3.28.3)
project(ngl)

include_directories(include)

add_library(stack src/stack.cpp include/stack.h)

add_library(interpreter src/interpreter/interpreter.cpp include/interpreter/interpreter.h)

add_executable(ngl src/ngl.cpp)