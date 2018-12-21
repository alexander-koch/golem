# Makefile for the golem programming language
# @Author Alexander Koch 2016
CC := gcc
MODULE_EXEC := golem
MODULE_IR := golem-ir
INC := -I.
PREFIX := /usr/local

# Run `make DEBUG=1` to build the debug version

# Linux:
# _msize is not available, set the -DNO_MEMINFO flag
# to disable the memory usage tracker.

# Golem flags / Options:
# Disable:
#   -DNO_AST	 	<-- Prints out the abstract syntax tree
#   -DNO_IR		 	<-- Prints out immediate representation (bytecode)
#   -DNO_EXEC		<-- Bytecode is not executed
#	-DNO_MEMINFO 	<-- Disables info on memory usage
# Enable:
#   -DTRACE	  		<-- While bytecode is executed, stack + instructions are printed
#   -DTRACE_STEP 	<-- When TRACE is set, step through every instruction
#   -DDB_VARS		<-- Debugs all variables by printing a message
#   -DDB_EVAL		<-- Debugs every ast evaluation

# C - compiler flags :: use c99
CFLAGS := -std=c99 -Wall -Wextra -Wno-unused-function -Wno-unused-parameter
LDFLAGS := -lm

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CFLAGS += -O2 -g
	MODULE_EXEC = golem-debug
else
	CFLAGS += -O3 -fno-gcse -fno-crossjumping -DNO_IR -DNO_MEMINFO -DNO_AST
endif

# All files for the compiler
FILES := adt/bytebuffer.c \
		adt/hashmap.c \
		adt/list.c \
		adt/vector.c \
		compiler/compiler.c \
		compiler/graphviz.c \
		compiler/scope.c \
		compiler/serializer.c \
		core/util.c \
		lexis/lexer.c \
		lib/corelib.c \
		lib/iolib.c \
		lib/mathlib.c \
		parser/ast.c \
		parser/parser.c \
		parser/types.c \
		vm/bytecode.c \
		vm/val.c \
		vm/vm.c

OBJDIR := bin
OBJ := $(FILES:.c=.o)
OBJECTS := $(addprefix $(OBJDIR)/, $(notdir $(OBJ)))

# Main executable
all: $(OBJDIR) $(OBJ) main.o
	@$(CC) $(LDFLAGS) $(OBJECTS) $(OBJDIR)/main.o -o $(MODULE_EXEC)

# Immediate representation tool / print .gvm bytecode
ir: $(OBJDIR) $(OBJ)
	@echo tools/ir.c
	@$(CC) $(CFLAGS) $(INC) -c tools/ir.c -o $(OBJDIR)/ir.o
	@$(CC) $(LDFLAGS) $(OBJECTS) $(OBJDIR)/ir.o -o $(MODULE_IR)

clean:
	rm -f $(OBJDIR)/*.o

%.o: %.c
	@echo $<
	@$(CC) $(CFLAGS) $(INC) -c $< -o $(OBJDIR)/$(notdir $@)

$(OBJDIR):
	@test -d $@ || mkdir $@

install: all
	install $(MODULE_EXEC) $(PREFIX)/bin

uninstall:
	rm $(PREFIX)/bin/$(MODULE_EXEC)

# Graphviz *.dot to *.svg
dot:
	dot -Tsvg -o ast.svg ast.dot

.PHONY: clean install uninstall
