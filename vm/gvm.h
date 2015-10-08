#ifndef vm_h
#define vm_h

#include <time.h>
#include <assert.h>
#include <stdint.h>

#include <core/api.h>
#include <adt/vector.h>
#include <adt/hashmap.h>

#include <vm/val.h>
#include <vm/gbytecode.h>

#define STACK_SIZE 512
#define LOCALS_SIZE 512

typedef struct
{
	// Stack
	val_t stack[STACK_SIZE];
	val_t locals[LOCALS_SIZE];
	int pc;
	int fp;
	int sp;
	int reserve;
	bool running;

	// Gargabe collection
	obj_t* firstVal;
	int numObjects;
	int maxObjects;
} vm_t;

// Methods
vm_t* vm_new();
void vm_run(vm_t* vm, vector_t* buffer);
void vm_free(vm_t* vm);

void push(vm_t* vm, val_t val);
val_t pop(vm_t* vm);

#endif