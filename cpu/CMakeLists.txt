cmake_minimum_required(VERSION 3.12)
project(cpu)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O0 -g -Wall -Wextra -pedantic -Weffc++ -Wno-unused-parameter")

add_executable(cpu main.cpp )
add_executable(assembler assembler.cpp)
add_executable(disassembler disassembler.cpp)