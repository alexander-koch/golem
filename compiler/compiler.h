/**
 * Compiler
 * @author Alexander Koch
 * @desc Compiles an abstract syntax tree (AST) into bytecode instructions.
 *
 * Scopes are managed, ASTs are optimized and converted to bytecode instructions.
 * An error is thrown by the compiler if the semantics are wrong.
 */

#ifndef compiler_h
#define compiler_h

#include <stdio.h>
#include <stdlib.h>
#include <core/mem.h>
#include <core/util.h>
#include <parser/parser.h>
#include <vm/bytecode.h>
#include <compiler/scope.h>
#include <lib/loadlib.h>

typedef struct compiler_t
{
	parser_t* parser;
	list_t* parsers;
	vector_t* buffer;
	scope_t* scope;
	vector_t* dlls;
	bool error;
	int depth;
} compiler_t;

void compiler_init(compiler_t* compiler);
vector_t* compile_buffer(compiler_t* compiler, const char* source, const char* name);
vector_t* compile_file(compiler_t* compiler, const char* filename);
void compiler_clear(compiler_t* compiler);
void compiler_free_symbols(compiler_t* compiler);
void compiler_dump(ast_t* node, int level);

#endif
