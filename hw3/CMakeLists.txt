cmake_minimum_required(VERSION 3.1)
project(vm_hw2)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-m32 -fstack-protector-all")

include_directories(include)

add_executable(VM_HW_2 src/main.cpp
        src/runtime.c
        src/bytefile.c
        src/interpretator.cpp)
