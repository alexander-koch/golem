#ifndef vm_h
#define vm_h

#include <stdio.h>
#include <stdlib.h>
#include <core/mem.h>
#include <parser/parser.h>
#include <vm/value.h>
#include <vm/scope.h>
#include <vm/bytecode.h>

typedef struct compiler_t
{
	parser_t parser;
} compiler_t;

list_t* compile_buffer(compiler_t* compiler, const char* source);
list_t* compile_file(compiler_t* compiler, const char* filename);
void buffer_free(list_t* buffer);

#endif
