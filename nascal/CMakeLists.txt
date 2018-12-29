cmake_minimum_required(VERSION 3.12)
project(nascal)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(nascal src/main.cpp include/ast.h include/parser.h)