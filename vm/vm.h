#ifndef vm_h
#define vm_h

#include <time.h>
#include <core/api.h>
#include <adt/stack.h>
#include <adt/vector.h>
#include <adt/hashmap.h>

#include <vm/bytecode.h>
#include <vm/value.h>

#define STACK_SIZE 512
#define LOCALS_SIZE 512

// Stack: classic stack for vm, push, pop values
// Locals: RAM for storing local variables
// pc: Program counter / instruction pointer, points to current instruction
// fp: frame pointer, used for functions (scope)
// sp: stack pointer, points to current memory location in stack

typedef struct
{
	// Stack
	value_t* stack[STACK_SIZE];
	value_t* locals[LOCALS_SIZE];
	int pc;
	int fp;
	int sp;
	int reserve;
	bool running;

	// Gargabe collection
	value_t* firstVal;
	int numObjects;
	int maxObjects;
} vm_t;

// External method definition
typedef struct FunctionDef
{
	const char* name;
	int (*func)(vm_t*);
} FunctionDef;

vm_t* vm_new();
void vm_run(vm_t* vm, vector_t* buffer);
void vm_free(vm_t* vm);

void push(vm_t* vm, value_t* val);
value_t* pop(vm_t* vm);

#endif
