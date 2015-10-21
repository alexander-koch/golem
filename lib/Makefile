# Makefile for the golem standard libraries
CC := gcc
INC := -I. -I..
FILES = ../vm/vm.c \
		../vm/bytecode.c \
		../vm/val.c \
		../parser/parser.c \
		../parser/ast.c \
		../adt/list.c \
		../adt/hashmap.c \
		../adt/vector.c \
		../core/util.c \
		../lexis/lexer.c \
		corelib.c \
		iolib.c \
		mathlib.c \
		loadlib.c


CFLAGS := -std=c99 -Wall -Wno-unused-function -Wno-unused-parameter -DDLL_EXPORT

all:
	$(CC) -shared -o libcore.dll $(INC) -DNO_MEMINFO  $(FILES) $(CFLAGS)
