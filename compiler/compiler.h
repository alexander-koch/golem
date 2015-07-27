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

// Symbol, info on class, variable or function
// variable => immutable / mutable + name + type
// function => name + returntype
// class => name + functions + variables
typedef struct symbol_t
{
	ast_t* node;
	int address;
	datatype_t type;
} symbol_t;

typedef struct scope_t
{
	hashmap_t* symbols;
	struct scope_t* super;
	list_t* subscopes;
	int address;
} scope_t;

typedef struct compiler_t
{
	parser_t parser;
	list_t* buffer;
	char* filename;
	bool debug;
	int address;
	bool error;
	scope_t* scope;
} compiler_t;

void compiler_init(compiler_t* compiler, bool debugmode);
list_t* compile_buffer(compiler_t* compiler, const char* source);
list_t* compile_file(compiler_t* compiler, const char* filename);
void compiler_clear(compiler_t* compiler);
void compiler_free_symbols(compiler_t* compiler);

#endif
