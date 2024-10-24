# 设置编译器必须在 project() 之前
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

cmake_minimum_required(VERSION 3.21)
project(AAA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable("coroutine-task"
        main.cpp
        io_utils.cpp)