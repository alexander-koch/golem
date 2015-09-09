// Compiler
// @desc Compiles source AST to a list of bytecode instructions
// @author Alexander Koch

#ifndef compiler_h
#define compiler_h

#include <stdio.h>
#include <stdlib.h>
#include <core/mem.h>
#include <core/util.h>
#include <parser/parser.h>
#include <vm/bytecode.h>
#include <compiler/scope.h>

// Compiler: manages scopes, converts every AST to multiple instructions
// Throws errors if wrong code is detected
// Also optimizes ASTs
typedef struct compiler_t
{
	parser_t* parser;
	vector_t* buffer;
	list_t* parsers;
	bool error;
	scope_t* scope;
	int depth;
} compiler_t;

void compiler_init(compiler_t* compiler);
vector_t* compile_buffer(compiler_t* compiler, const char* source, const char* name);
vector_t* compile_file(compiler_t* compiler, const char* filename);
void compiler_clear(compiler_t* compiler);
void compiler_free_symbols(compiler_t* compiler);

#endif
