CC = gcc
MODULE = golem
INC = -I.

CFLAGS = -std=c99 -Wall -Wno-unused-function -Wno-unused-parameter -DCODE_OPT -DNO_TRACE #-DNO_AST

# Options:
# Disable:
# -DNO_AST   <-- Prints out the abstract syntax tree
# -DNO_IR	 <-- Prints out immediate representation (bytecode)
# -DNO_TRACE <-- While bytecode is executed, stack + instructions are printed
# -DNO_EXEC  <-- Bytecode is not executed
# Enable:
# -DDB_VARS	 <-- Debugs all variables by printing a message
# -DB_EVAL	 <-- Debugs every ast evaluation
# -DCODE_OPT <-- Optimizes bytecode

FILES = main.c \
		core/api.c \
		core/util.c \
		adt/queue.c \
		adt/stack.c \
		adt/list.c \
		adt/hashmap.c \
		lexis/lexer.c \
		parser/ast.c \
		parser/parser.c \
		parser/optimizer.c \
		compiler/compiler.c \
		compiler/asm_compiler.c \
		compiler/bytecode_writer.c \
		vm/value.c \
		vm/bytecode.c \
		vm/vm.c \
		lib/iolib.c \
		lib/strlib.c \
		lib/mathlib.c \
		lib/ginit.c


debug:
	$(CC) $(CFLAGS) $(INC) -c $(FILES)
	$(CXX) *.o $(LDFLAGS) -o $(MODULE)
	-rm *.o

asm:
	nasm -fwin64 out.asm
	gcc out.obj
	rm -f out.obj
	#rm -f out.asm
