cmake_minimum_required(VERSION 3.1)
project(vm_hw2)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(VM_HW src/main.cpp
        src/runtime.c
        src/bytefile.c
        src/interpreter.cpp)
