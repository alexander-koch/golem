CC = gcc
INC = -I.
CFLAGS = -std=c99
MODULE = nemesis
FILES = main.c \
		core/api.c \
		core/util.c \
		adt/queue.c \
		adt/stack.c \
		adt/list.c \
		adt/hashmap.c \
		lexic/lexer.c \
		vm/vm.c \
		vm/scope.c \
		vm/value.c \
		ast.c \
		parser.c

debug:
	$(CC) -g -o $(MODULE) $(INC) $(FILES) -Wall $(CFLAGS)

release:
	$(CC) -g -o $(MODULE) $(INC) $(FILES) -Wall $(CFLAGS) -O3 -Wextra
