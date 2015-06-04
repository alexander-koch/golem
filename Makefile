CC = gcc
MODULE = nemesis
INC = -I.
CFLAGS = -std=c99

FILES = main.c \
		core/api.c \
		core/util.c \
		core/value.c \
		adt/queue.c \
		adt/stack.c \
		adt/list.c \
		adt/hashmap.c \
		lexis/lexer.c \
		parser/ast.c \
		parser/parser.c \
		compiler/compiler.c \
		compiler/bytecode.c \
		vm/vm.c

debug:
	$(CC) -g -o $(MODULE) $(INC) $(FILES) $(LIBS) -Wall $(CFLAGS)

release:
	$(CC) -g -o $(MODULE) $(INC) $(FILES) $(LIBS) -Wall $(CFLAGS) -O3 -Wextra
