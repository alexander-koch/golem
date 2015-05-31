#ifndef compiler_h
#define compiler_h

#include <stdio.h>
#include <stdlib.h>
#include <core/mem.h>
#include <core/value.h>
#include <parser/parser.h>
#include <compiler/bytecode.h>

typedef struct compiler_t
{
	parser_t parser;
} compiler_t;

list_t* compile_buffer(compiler_t* compiler, const char* source);
list_t* compile_file(compiler_t* compiler, const char* filename);
void buffer_free(list_t* buffer);

#endif
