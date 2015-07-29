CC = gcc
MODULE = golem
INC = -I.

CFLAGS = -std=c99 -Wall -Wno-unused-function -Wno-unused-parameter -DNO_TRACE

# Options:
# -DNO_IR	 <-- Prints out immediate representation (bytecode)
# -DNO_TRACE <-- While bytecode is executed, stack + instructions are printed
# -DNO_EXEC  <-- Bytecode is not executed

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
		vm/value.c \
		vm/bytecode.c \
		vm/vm.c

debug:
	$(CC) $(CFLAGS) $(INC) -c $(FILES)
	$(CXX) *.o $(LDFLAGS) -o $(MODULE)
	-rm *.o

bitcode:
	llvm-dis out.bc

asm:
	llc out.bc -o out.s
