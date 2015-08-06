// Compiler
// @desc Compiles source AST to a list of bytecode instructions
// @author Alexander Koch

#ifndef compiler_h
#define compiler_h

#include <stdio.h>
#include <stdlib.h>
#include <core/mem.h>
#include <parser/parser.h>
#include <vm/bytecode.h>
#include <adt/hashmap.h>

// Symbol is a certain info for the compiler,
// can be a function, variable, class, etc.
// everything that has to be identified.
// variable => immutable / mutable + name + type
// function => name + returntype
// class    => name + functions + variables
typedef struct symbol_t
{
	ast_t* node;
	int address;
	datatype_t type;
	bool global;
} symbol_t;

// Scope: contains symbols
typedef struct scope_t
{
	hashmap_t* symbols;
	struct scope_t* super;
	list_t* subscopes;
	int address;
} scope_t;

// Compiler: manages scopes, converts every AST to multiple instructions
// Throws errors if wrong code is detected
// Also optimizes ASTs
typedef struct compiler_t
{
	parser_t parser;
	list_t* buffer;
	char* filename;
	int address;
	bool error;
	scope_t* scope;
	int depth;
} compiler_t;

void compiler_init(compiler_t* compiler);
list_t* compile_buffer(compiler_t* compiler, const char* source);
list_t* compile_file(compiler_t* compiler, const char* filename);
void compiler_clear(compiler_t* compiler);
void compiler_free_symbols(compiler_t* compiler);

#endif
