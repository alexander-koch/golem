#ifndef vm_h
#define vm_h

#include <core/api.h>
#include <adt/stack.h>
#include <adt/list.h>
#include <adt/hashmap.h>

#include <compiler/bytecode.h>
#include <core/value.h>

typedef struct
{
	stack_t* stack;
	hashmap_t* fields;
	U64 pc;
} vm_t;

typedef struct
{
	char* name;
	bool mutate;
	value_t* val;
} variable_t;

vm_t* vm_new();
void vm_process(vm_t* vm, list_t* buffer);
void vm_execute(vm_t* vm, list_t* buffer);
void vm_free(vm_t* vm);

#endif
