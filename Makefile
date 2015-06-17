CC = gcc
MODULE = golem
INC = -I.

LLVMC = -IC:\llvm\build\include -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS #`llvm-config --cflags`
LLVMLD = `llvm-config --cxxflags --ldflags --libs core analysis native bitwriter --system-libs`

CFLAGS = -std=c99 -Wall -Wno-unused-function -Wno-unused-parameter #$(LLVMC) -D__USE_LLVM__
LDFLAGS = #$(LLVMLD)

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
		parser/optimizer.c \
		compiler/compiler.c \
		compiler/bytecode.c \
		vm/vm.c

debug:
	#mkdir -p bin/
	$(CC) $(CFLAGS) $(INC) -c $(FILES)
	$(CXX) *.o $(LDFLAGS) -o $(MODULE)
	-rm *.o

release:
	$(CC) $(CFLAGS) -g -o $(MODULE) $(INC) $(FILES) $(LIBS) $(LDFLAGS) -O3 -Wextra
