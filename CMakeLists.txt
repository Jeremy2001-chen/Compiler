cmake_minimum_required(VERSION 3.20)
project(grammar_1005)

set(CMAKE_CXX_STANDARD 14)

add_executable(grammar_1005 main.cpp grammar.h lexical.h word.h output.h)
