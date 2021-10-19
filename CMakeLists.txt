cmake_minimum_required(VERSION 3.20)
project(Compiler)

set(CMAKE_CXX_STANDARD 14)

add_executable(Compiler main.cpp grammar.h lexical.h word.h output.h error.h lib/node.h lib/binary_exp.h)
