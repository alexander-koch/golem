CC = gcc
MODULE = golem
INC = -I.

CFLAGS = -std=c99 -Wall -Wno-unused-function -Wno-unused-parameter -DNO_EXPERIMENTAL -DNO_TRACE #-DNO_AST

# Options:
# Disable:
# -DNO_AST   <-- Prints out the abstract syntax tree
# -DNO_IR	 <-- Prints out immediate representation (bytecode)
# -DNO_TRACE <-- While bytecode is executed, stack + instructions are printed
# -DNO_EXEC  <-- Bytecode is not executed
# Enable:
# -DDB_VARS	 <-- Debugs all variables by printing a message
# -DDB_EVAL	 <-- Debugs every ast evaluation

FILES = main.c \
		core/api.c \
		core/util.c \
		adt/queue.c \
		adt/stack.c \
		adt/list.c \
		adt/vector.c \
		adt/hashmap.c \
		lexis/lexer.c \
		parser/ast.c \
		parser/parser.c \
		compiler/compiler.c \
		compiler/scope.c \
		compiler/bytecode_writer.c \
		vm/value.c \
		vm/bytecode.c \
		vm/vm.c \
		vm/jit.c \
		lib/corelib.c \
		lib/strlib.c \
		lib/mathlib.c

debug:
	$(CC) -O2 $(CFLAGS) $(INC) $(FILES) $(LDFLAGS) -g -o $(MODULE)

release:
	$(CC) -O3 $(CFLAGS) -DNO_IR -DNO_TIME -DNO_MEMINFO $(INC) $(FILES) $(LDFLAGS) -o $(MODULE)

run:
	gdb golem

old:
	$(CC) $(CFLAGS) $(INC) -c $(FILES)
	$(CC) *.o $(LDFLAGS) -o $(MODULE)
	rm *.o

asm:
	nasm -fwin64 out.asm
	gcc out.obj
	rm out.obj
