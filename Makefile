CC = gcc
INC = -I.
LIBS =
MODULE = nemesis
FILES = main.c \
		core/api.c \
		core/util.c \
		adt/queue.c \
		adt/stack.c \
		adt/list.c \
		lexic/lexer.c \
		ast.c \
		parser.c \
		vm.c

debug:
	$(CC) -g -o $(MODULE) $(INC) $(FILES) -Wall $(LIBS)

release:
	$(CC) -g -o $(MODULE) $(INC) $(FILES) -Wall $(LIBS) -O3 -Wextra
