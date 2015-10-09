#include "gvm.h"

void gc(vm_t* vm);

vm_t* vm_new()
{
	vm_t* vm = malloc(sizeof(*vm));
	vm->pc = 0;
	vm->fp = 0;
	vm->sp = 0;
	vm->reserve = 0;
	vm->running = false;
	vm->firstVal = 0;
	vm->numObjects = 0;
	vm->maxObjects = 8;
	return vm;
}

void vm_throw(vm_t* vm, const char* format, ...)
{
    printf("=> Exception thrown: ");
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
    printf("\nat: PC(%d), SP(%d), FP(%d)\n", vm->pc, vm->sp, vm->fp);
	vm->running = false;
}

void mark(val_t v)
{
	if(IS_OBJ(v))
	{
		obj_t* obj = AS_OBJ(v);
		if(!obj->marked) {
			obj->marked = 1;
		}
	}
}

void markAll(vm_t* vm)
{
	for(int i = 0; i < vm->sp; i++)
	{
		mark(vm->stack[i]);
	}
	for(int i = 0; i < LOCALS_SIZE; i++)
	{
		mark(vm->locals[i]);
	}
}

void sweep(vm_t* vm)
{
	obj_t** val = &vm->firstVal;
	while(*val)
	{
		if(!(*val)->marked)
		{
			obj_t* unreached = *val;
			*val = unreached->next;
			obj_free(unreached);
			unreached = 0;
			vm->numObjects--;
		}
		else
		{
			(*val)->marked = 0;
			val = &(*val)->next;
		}
	}
}

void gc(vm_t* vm)
{
// Garbage day!
#ifdef TRACE
	console("Collecting garbage...\n");
#endif

	markAll(vm);
	sweep(vm);
	vm->maxObjects = vm->numObjects * 2;
}

void push(vm_t* vm, val_t val)
{
	if(vm->sp >= STACK_SIZE)
	{
		vm_throw(vm, "Stack overflow");
		return;
	}

	vm->stack[vm->sp] = val;
	vm->sp++;
}

val_t pop(vm_t* vm)
{
	vm->sp--;
	val_t v = vm->stack[vm->sp];
	vm->stack[vm->sp] = 0;
	return v;
}

void obj_append(vm_t* vm, obj_t* obj)
{
	if(vm->numObjects == vm->maxObjects)
	{
		gc(vm);
	}

	obj->marked = 0;
	obj->next = vm->firstVal;
	vm->firstVal = obj;
	vm->numObjects++;
}

void vm_register(vm_t* vm, val_t v1)
{
	push(vm, v1);
	if(IS_OBJ(v1))
	{
		obj_append(vm, AS_OBJ(v1));
	}
}

// Fetches the next instruction
instruction_t* vm_fetch(vm_t* vm, vector_t* buffer)
{
	return vector_get(buffer, vm->pc);
}

// Just prints out instruction codes
void vm_print_code(vm_t* vm, vector_t* buffer)
{
	vm->pc = 0;
	console("\nImmediate code:\n");

	instruction_t* instr = vm_fetch(vm, buffer);
	while(instr->op != OP_HLT)
	{
		console("  %.2d: %s", vm->pc, op2str(instr->op));
		if(instr->v1 != NULL_VAL)
		{
			console(", ");
			val_print(instr->v1);
		}
		if(instr->v2 != NULL_VAL)
		{
			console(", ");
			val_print(instr->v2);
		}
		console("\n");

		vm->pc++;
		instr = vm_fetch(vm, buffer);
	}
	vm->pc = 0;
}

void reserve(vm_t* vm, size_t args)
{
	if(vm->reserve > 0)
	{
		for(size_t i = 1; i <= args; i++)
		{
			vm->stack[vm->sp+vm->reserve-i] = vm->stack[vm->sp-i];
		}
		vm->sp += vm->reserve;
	}
}

// Processes a buffer instruction based on instruction / program counter (pc).
void vm_process(vm_t* vm, instruction_t* instr)
{

#ifdef TRACE
	console("  %.2d (SP:%.2d, FP:%.2d): %s", vm->pc, vm->sp, vm->fp, op2str(instr->op));
	if(instr->v1 != NULL_VAL)
	{
		console(", ");
		val_print(instr->v1);
	}
	if(instr->v2 != NULL_VAL)
	{
		console(", ");
		val_print(instr->v2);
	}
	console(" => STACK [");
	// int begin = vm->sp - 5;
	// if(begin < 0) begin = 0;

	for(int i = 0; i < vm->sp; i++)
	{
		if(vm->stack[i] != NULL_VAL)
		{
			val_print(vm->stack[i]);
			if(i < vm->sp-1) console(", ");
		}
	}
	console("]\n");
#endif

	switch(instr->op)
	{
		case OP_PUSH:
		{
			vm_register(vm, instr->v1);
			break;
		}
		case OP_POP:
		{
			pop(vm);
			break;
		}
		case OP_HLT:
		{
			vm->running = false;
			break;
		}
		case OP_STORE:
		{
			int offset = AS_NUM(instr->v1);
			if(offset < 0)
			{
				vm->stack[vm->fp+offset] = pop(vm);
			}
			else
			{
				vm->locals[vm->fp+offset] = pop(vm);
			}
			break;
		}
		case OP_LOAD:
		{
			int offset = AS_NUM(instr->v1);
			if(offset < 0)
			{
				push(vm, vm->stack[vm->fp+offset]);
			}
			else
			{
				push(vm, vm->locals[vm->fp+offset]);
			}
			break;
		}
		case OP_GSTORE:
		{
			int offset = AS_NUM(instr->v1);
			val_free(vm->locals[offset]);
			vm->locals[offset] = pop(vm);
			break;
		}
		case OP_GLOAD:
		{
			int offset = AS_NUM(instr->v1);
			val_t v = vm->locals[offset];
			push(vm, v);
			break;
		}
		case OP_LDARG0:
		{
			size_t args = AS_NUM(vm->stack[vm->fp-3]);
			push(vm, vm->stack[vm->fp-args-4]);
			break;
		}
		case OP_SETARG0:
		{
			size_t args = AS_NUM(vm->stack[vm->fp-3]);
			vm->stack[vm->fp-args-4] = pop(vm);
			break;
		}
		case OP_UPVAL:
		{
			int scopes = AS_NUM(instr->v1);
			int offset = AS_NUM(instr->v2);
			int fp = vm->fp;
			int sp = vm->sp;

			for(int i = 0; i < scopes; i++)
			{
				vm->fp = AS_NUM(vm->stack[vm->fp - 2]);
			}
			vm->sp = vm->fp;

			val_t val = (offset < 0) ? vm->stack[vm->fp+offset] : vm->locals[vm->fp+offset];
			vm->sp = sp;
			vm->fp = fp;
			push(vm, val);
			break;
		}
		case OP_UPSTORE:
		{
			val_t newVal = pop(vm);

			int scopes = AS_NUM(instr->v1);
			int offset = AS_NUM(instr->v2);

			int fp = vm->fp;
			int sp = vm->sp;

			for(int i = 0; i < scopes; i++)
			{
				vm->fp = AS_NUM(vm->stack[vm->fp - 2]);
			}
			vm->sp = vm->fp;

			if(offset < 0)
			{
				vm->stack[vm->fp+offset] = newVal;
			}
			else
			{
				val_free(vm->locals[vm->fp+offset]);
				vm->locals[vm->fp+offset] = newVal;
			}

			vm->sp = sp;
			vm->fp = fp;
			break;
		}
		case OP_SYSCALL:
		{
			// TODO: Improve
			char* name = AS_STRING(AS_OBJ(instr->v1));

			if(!strcmp(name, "print"))
			{
				val_print(pop(vm));
			}
			else if(!strcmp(name, "println"))
			{
				val_print(pop(vm));
				printf("\n");
			}

			break;
		}
		case OP_INVOKE:
		{
			// Arguments already on the stack
			int address = AS_NUM(instr->v1);
			size_t args = AS_NUM(instr->v2);

			// Arg0 -3
			// Arg1 -2
			// Arg2 -1
			// sp (args=3) (reserve=2)
			// ...  +1
			// ...  +2
			reserve(vm, args);

			push(vm, NUM_VAL(args));
			push(vm, NUM_VAL(vm->fp));
			push(vm, NUM_VAL(vm->pc));

			// |   STACK_BOTTOM	  |
			// |...				  |
			// |Arg0			-7|
			// |Arg1			-6|
			// |Arg2			-5|
			// |...				-4|
			// |NUM_ARGS		-3|
			// |FP				-2|	<-- vm->fp - 2
			// |PC				-1|
			// |...				 0|	<-- current position sp / fp
			// |...				+1|
			// |    STACK_TOP     |

			vm->fp = vm->sp;
			vm->pc = address;
			return;
		}
		case OP_INVOKEVIRTUAL:
		{
			int address = AS_NUM(instr->v1);
			size_t args = AS_NUM(instr->v2);

			if(args > 0) reserve(vm, args+1); // <-- Causes memory leak but fixes problem, why?!?
			push(vm, NUM_VAL(args));
			push(vm, NUM_VAL(vm->fp));
			push(vm, NUM_VAL(vm->pc));
			vm->fp = vm->sp;
			vm->pc = address;
			return;
		}
		case OP_RESERVE:
		{
			// Reserves some memory.
			// Uses this before any function call to maintain stored values.
			// Otherwise, if too many locals are saved in the last scope e.g. from 1 - 11,
			// the new scope overwrites the old values for instance: fp at 5, overwrites 5 - 11.

			vm->reserve = AS_NUM(instr->v1);
			break;
		}
		case OP_RET:
		{
			// Returns to previous instruction pointer,
			// and pushes the return value back on the stack.
			// If you call this function make sure there is a return value on the stack.
			val_t ret = pop(vm);

			vm->sp = vm->fp;
			vm->pc = AS_NUM(pop(vm));
			vm->fp = AS_NUM(pop(vm));
			size_t args = AS_NUM(pop(vm));

			vm->sp -= args;
			push(vm, ret);
			break;
		}
		case OP_RETVIRTUAL:
		{
			// Returns from a virtual class function
			val_t ret = pop(vm);

			vm->sp = vm->fp;
			vm->pc = AS_NUM(pop(vm));
			vm->fp = AS_NUM(pop(vm));
			size_t args = AS_NUM(pop(vm));

			vm->sp -= args;
			val_t clazz = pop(vm);
			push(vm, ret);
			push(vm, clazz);
			break;
		}
		case OP_JMP:
		{
			vm->pc = AS_NUM(instr->v1);
			return;
		}
		case OP_JMPF:
		{
			bool result = AS_BOOL(pop(vm));
			if(!result)
			{
				vm->pc = AS_NUM(instr->v1);
				return;
			}
			break;
		}


		// Missing
		// TODO: Convert IADD and FADD to ADD
		// There is no need for different operators for one internal type

		case OP_IADD:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 + v2));
			break;
		}
		case OP_ISUB:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 - v2));
			break;
		}
		case OP_IMUL:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 * v2));
			break;
		}
		case OP_IDIV:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 / v2));
			break;
		}
		case OP_MOD:
		{
			int v2 = AS_NUM(pop(vm));
			int v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 % v2));
			break;
		}
		case OP_BITL:
		{
			int v2 = AS_NUM(pop(vm));
			int v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 << v2));
			break;
		}
		case OP_BITR:
		{
			int v2 = AS_NUM(pop(vm));
			int v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 >> v2));
			break;
		}
		case OP_BITAND:
		{
			int v2 = AS_NUM(pop(vm));
			int v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 & v2));
			break;
		}
		case OP_BITOR:
		{
			int v2 = AS_NUM(pop(vm));
			int v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 | v2));
			break;
		}
		case OP_BITXOR:
		{
			int v2 = AS_NUM(pop(vm));
			int v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(v1 ^ v2));
			break;
		}
		case OP_BITNOT:
		{
			int v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(~v1));
			break;
		}
		case OP_IMINUS:
		{
			int v1 = AS_NUM(pop(vm));
			push(vm, NUM_VAL(-v1));
			break;
		}

		// Missing

		case OP_LT:
		{
			double v2 = AS_NUM(pop(vm));
			double v1 = AS_NUM(pop(vm));
			push(vm, BOOL_VAL(v1 < v2));
			break;
		}
		default:
		{
			printf("Uknown operator encountered\n");
			break;
		}
	}

	vm->pc++;
}

// Executes a buffer / list of instructions
void vm_run(vm_t* vm, vector_t* buffer)
{
	// Reset vm
	vm->sp = 0;
	vm->pc = 0;
	vm->fp = 0;
	vm->reserve = 0;
	vm->running = true;
	//memset(vm->stack, NULL_VAL, sizeof(val_t) * STACK_SIZE);
	//memset(vm->locals, NULL_VAL, sizeof(val_t) * LOCALS_SIZE);

#ifndef NO_IR
	// Print out bytecodes
	vm_print_code(vm, buffer);
	console("\nExecution:\n");
#endif

	// Run
#ifndef NO_EXEC
#ifndef NO_TIME
	clock_t begin = clock();
#endif

	while(vm->running)
	{
		vm_process(vm, vm_fetch(vm, buffer));
	}

#ifndef NO_TIME
	clock_t end = clock();
	double elapsed = (double)(end - begin) / CLOCKS_PER_SEC;
	console("Elapsed time: %.5f (sec)\n", elapsed);
#endif
#endif

	// Move stack pointer to zero, -> clears all elements by gc
	// Discard the rest
	vm->sp = 0;
	gc(vm);

	// Clear
	for(int i = 0; i < LOCALS_SIZE; i++)
	{
		val_free(vm->locals[i]);
		vm->locals[i] = NULL_VAL;
	}
}

// Frees the memory used by the vm
void vm_free(vm_t* vm)
{
	free(vm);
}
