#ifndef vm_h
#define vm_h

#include <core/api.h>
#include <adt/stack.h>
#include <adt/list.h>
#include <adt/hashmap.h>

#include <vm/bytecode.h>
#include <vm/value.h>

#define STACK_SIZE 1024
#define LOCALS_SIZE 1024

typedef struct
{
	value_t* stack[STACK_SIZE];
	value_t* locals[LOCALS_SIZE];
	int pc;
	int fp;
	int sp;
	int numObjects;
	int maxObjects;
} vm_t;

vm_t* vm_new();
instruction_t* vm_peek(vm_t* vm, list_t* buffer);
void vm_process(vm_t* vm, list_t* buffer);
void vm_execute(vm_t* vm, list_t* buffer);
void vm_free(vm_t* vm);

#endif
