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
	bool error;
	stack_t* stack;
	int pc;
	int fp;
} vm_t;

vm_t* vm_new();
instruction_t* vm_peek(vm_t* vm, list_t* buffer);
void vm_process(vm_t* vm, list_t* buffer);
void vm_execute(vm_t* vm, list_t* buffer);
void vm_free(vm_t* vm);

#endif
