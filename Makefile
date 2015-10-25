# Makefile for the golem programming language
CC := gcc
MODULE := golem
INC := -I.

# Golem flags
# Add flags explained below for debugging features
GFLAGS := #-DTRACE #-DTRACE_STEP #-DNO_AST

# GFLAGS / Options:
# Disable:
#   -DNO_AST     <-- Prints out the abstract syntax tree
#   -DNO_IR	     <-- Prints out immediate representation (bytecode)
#   -DNO_EXEC    <-- Bytecode is not executed
#   -DNO_TIME    <-- Disables execution time printing
# 	-DNO_MEMINFO <-- Disables info on memory usage
# Enable:
#   -DTRACE 	 <-- While bytecode is executed, stack + instructions are printed
#   -DTRACE_STEP <-- When TRACE is set, step through every instruction
#   -DDB_VARS    <-- Debugs all variables by printing a message
#   -DDB_EVAL    <-- Debugs every ast evaluation

# C - compiler flags :: use c99
CFLAGS := -std=c99 -Wall -Wno-unused-function -Wno-unused-parameter $(GFLAGS)

# All files for the compiler
FILES := main.c \
		core/util.c \
		adt/queue.c \
		adt/stack.c \
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
		lib/iolib.c \
		lib/loadlib.c

# Debugging version (default)
debug:
	$(CC) -O2 $(CFLAGS) $(INC) $(FILES) $(LDFLAGS) -g -o $(MODULE)

# Final release version
release:
	$(CC) -O3 -fno-gcse $(CFLAGS) -DNO_IR -DNO_MEMINFO -DNO_AST $(INC) $(FILES) $(LDFLAGS) -o $(MODULE)

# Experimental dll-library feature
libs:
	make -C ./lib MAKEFLAGS=

# Graphviz *.dot to *.png
dot:
	dot -Tpng -o ast.png ast.dot
	rm ast.dot

dotsvg:
	dot -Tsvg -o ast.svg ast.dot
	rm ast.dot

# git undo (in case of emergency)
gitUndo:
	git reset --soft HEAD~1
