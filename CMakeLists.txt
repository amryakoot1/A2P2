cmake_minimum_required(VERSION 3.20)
project(vole_code)

set(CMAKE_CXX_STANDARD 17)

add_executable(vole_code
        A2-S7,S8-20242441-20250456.cpp
        Hardware.cpp
        Instructions.cpp
        CPU.cpp
        Machine.cpp
)