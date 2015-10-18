# Makefile for the golem standard libraries
CC := gcc
INC := -I. -I..
FILES = ../vm/vm.c \
		../vm/bytecode.c \
		../vm/value.c \
		../parser/parser.c \
		../parser/ast.c \
		../adt/list.c \
		../adt/hashmap.c \
		../adt/vector.c \
		../core/api.c \
		../core/util.c \
		../lexis/lexer.c \
		loadlib.c


CFLAGS := -std=c99 -Wall -Wno-unused-function -Wno-unused-parameter -DDLL_EXPORT

core:
	$(CC) -shared -o libcore.dll $(INC) corelib.c $(FILES) $(CFLAGS)

math:
	$(CC) -shared -o libmath.dll $(INC) mathlib.c $(FILES) $(CFLAGS)
