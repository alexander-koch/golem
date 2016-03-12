# Makefile for the golem programming language
# @Author Alexander Koch 2015
CC := gcc
MODULE_RELEASE := golem
MODULE_DEBUG := golem-debug
MODULE_IR := golem-ir
INC := -I.

# Linux:
# To build on linux, add -lm at the end.
# Also _msize is not available, so set the
# -DNO_MEMINFO flag to disable the memory usage tracker.

# Golem flags
# Add flags explained below for debugging features
GFLAGS := #-DTRACE #-DTRACE_STEP #-DNO_AST

# GFLAGS / Options:
# Disable:
#   -DNO_AST     <-- Prints out the abstract syntax tree
#   -DNO_IR	     <-- Prints out immediate representation (bytecode)
#   -DNO_EXEC    <-- Bytecode is not executed
# 	-DNO_MEMINFO <-- Disables info on memory usage
# Enable:
#   -DTRACE 	 <-- While bytecode is executed, stack + instructions are printed
#   -DTRACE_STEP <-- When TRACE is set, step through every instruction
#   -DDB_VARS    <-- Debugs all variables by printing a message
#   -DDB_EVAL    <-- Debugs every ast evaluation

# C - compiler flags :: use c99
CFLAGS := -std=c99 -Wall -Wno-unused-function -Wno-unused-parameter $(GFLAGS)

# All files for the compiler
FILES := core/util.c \
		adt/list.c \
		adt/vector.c \
		adt/hashmap.c \
		adt/bytebuffer.c \
		lexis/lexer.c \
		parser/ast.c \
		parser/parser.c \
		compiler/serializer.c \
		compiler/compiler.c \
		compiler/scope.c \
		compiler/graphviz.c \
		compiler/doc.c \
		vm/val.c \
		vm/bytecode.c \
		vm/vm.c \
		lib/corelib.c \
		lib/mathlib.c \
		lib/iolib.c

all: debug release ir

# Debug version
debug:
	$(CC) -O2 $(CFLAGS) main.c $(INC) $(FILES) -g -o $(MODULE_DEBUG)

# Final release version
release:
	$(CC) -O3 -fno-gcse -fno-crossjumping $(CFLAGS) -DNO_IR -DNO_MEMINFO -DNO_AST main.c $(INC) $(FILES) -o $(MODULE_RELEASE)

# Immediate representation tool / print .gvm bytecode
ir:
	$(CC) -O3 $(CFLAGS) -DNO_MEMINFO ir.c  $(INC) $(FILES) -o $(MODULE_IR)

# Graphviz *.dot to *.svg
dot:
	dot -Tsvg -o ast.svg ast.dot
