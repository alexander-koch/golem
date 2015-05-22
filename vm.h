#ifndef vm_h
#define vm_h

#include <stdio.h>
#include <stdlib.h>
#include <core/mem.h>
#include <parser.h>

typedef struct vm_t
{
	parser_t parser;
} vm_t;

void vm_run_buffer(vm_t* vm, const char* source);
int vm_run_file(vm_t* vm, const char* filename);

#endif
