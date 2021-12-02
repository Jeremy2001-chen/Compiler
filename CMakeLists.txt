cmake_minimum_required(VERSION 3.20)
project(Compiler)

set(CMAKE_CXX_STANDARD 14)

add_executable(Compiler main.cpp grammar.h lexical.h word.h output.h error.h lib/node.h lib/binary_exp.h lib/single_exp.h lib/single_exp.h lib/value.h lib/function.h lib/block.h table.h lib/comp_unit.h type.h ir/ir_table.h ir/ir.h ir/ir_code.h mips/mips_table.h mips/mips_table.h mips/mips.h mips/mips_output.h list/mylist.h)
