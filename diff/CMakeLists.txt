cmake_minimum_required(VERSION 3.12)
project(diff)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(diff src/main.cpp src/biginteger.cpp src/rational.cpp src/parser.cpp src/expression.cpp)