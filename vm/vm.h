#ifndef vm_h
#define vm_h

#include <core/api.h>
#include <adt/stack.h>
#include <adt/list.h>
#include <adt/hashmap.h>

#include <vm/bytecode.h>
#include <vm/value.h>

typedef struct
{
	stack_t* stack;
	hashmap_t* fields;
	hashmap_t* functions;
	U64 pc;
	bool error;
} vm_t;

typedef struct
{
	char* name;
	bool mutate;
	value_t* val;
} variable_t;

typedef struct
{
	char* name;
	size_t args;
	U64 pc;
	list_t* params;
} function_t;

vm_t* vm_new();
instruction_t* vm_peek(vm_t* vm, list_t* buffer);
void vm_process(vm_t* vm, list_t* buffer);
void vm_execute(vm_t* vm, list_t* buffer);
void vm_free(vm_t* vm);

#endif
