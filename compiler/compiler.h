#ifndef compiler_h
#define compiler_h

#include <stdio.h>
#include <stdlib.h>
#include <core/mem.h>
#include <parser/parser.h>
#include <vm/bytecode.h>
#include <adt/hashmap.h>

#ifdef __USE_LLVM__
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <compiler/llvm_compiler.h>
#endif

typedef struct compiler_t
{
	parser_t parser;
	list_t* buffer;
	char* filename;
	bool debug;
	hashmap_t* symbols;
	int address;
} compiler_t;

void compiler_init(compiler_t* compiler);
list_t* compile_buffer(compiler_t* compiler, const char* source);
list_t* compile_file(compiler_t* compiler, const char* filename);
void compiler_clear(compiler_t* compiler);
void compiler_free_symbols(compiler_t* compiler);

#endif
