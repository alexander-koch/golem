#ifndef vm_h
#define vm_h

#include <stdio.h>
#include <stdlib.h>
#include <core/mem.h>
#include <vm/value.h>
#include <vm/scope.h>
#include <parser.h>

typedef struct vm_t
{
	parser_t parser;
	scope_t* scope;
} vm_t;

void vm_init(vm_t* vm);
void vm_run_buffer(vm_t* vm, const char* source);
int vm_run_file(vm_t* vm, const char* filename);
void vm_free(vm_t* vm);

#endif
