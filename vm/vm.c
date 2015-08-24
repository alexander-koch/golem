#include "vm.h"

// Standard Library
// TODO:
// something like binary find function in dll
// =>
// int golem_println(VARARGS) {
//		pop();
//
//}
// Return -1 if fail

extern int core_print(vm_t* vm);
extern int core_println(vm_t* vm);
extern int core_getline(vm_t* vm);
extern int core_f2i(vm_t* vm);
extern int core_i2f(vm_t* vm);
extern int core_c2i(vm_t* vm);
extern int core_parseFloat(vm_t* vm);

static FunctionDef system_methods[] = {
	{"print", core_print},
	{"println", core_println},
	{"getline", core_getline},
	{"f2i", core_f2i},
	{"i2f", core_i2f},
	{"c2i", core_c2i},
	{"parseFloat", core_parseFloat},
	{0, 0}	/* sentinel */
};

vm_t* vm_new()
{
	vm_t* vm = malloc(sizeof(*vm));
	vm->pc = 0;
	vm->fp = 0;
	vm->sp = 0;
	vm->firstVal = 0;
	vm->numObjects = 0;
	vm->maxObjects = 8;
	return vm;
}

void mark(value_t* v)
{
	if(!v->marked)
	{
		v->marked = 1;
	}
}

void markAll(vm_t* vm)
{
	for(int i = 0; i < vm->sp; i++)
	{
		if(vm->stack[i]) mark(vm->stack[i]);
	}
}

void sweep(vm_t* vm)
{
	value_t** val = &vm->firstVal;
	while(*val)
	{
		if(!(*val)->marked)
		{
			value_t* unreached = *val;
			*val = unreached->next;
			value_free(unreached);
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
#ifndef NO_TRACE
	console("Collecting garbage...\n");
#endif

	markAll(vm);
	sweep(vm);
	vm->maxObjects = vm->numObjects * 2;
}

void push(vm_t* vm, value_t* val)
{
	if(vm->numObjects == vm->maxObjects)
	{
		gc(vm);
	}

	vm->stack[vm->sp] = val;
	vm->sp++;

	val->marked = 0;
	val->next = vm->firstVal;
	vm->firstVal = val;
	vm->numObjects++;
}

value_t* pop(vm_t* vm)
{
	vm->sp--;
	value_t* v = vm->stack[vm->sp];
	vm->stack[vm->sp] = 0;
	return v;
}

instruction_t* vm_peek(vm_t* vm, vector_t* buffer)
{
	if(vm->pc >= vector_size(buffer)) return 0;
	return vector_get(buffer, vm->pc);
}

// Just prints out instruction codes
void vm_print_code(vm_t* vm, vector_t* buffer)
{
	console("\nImmediate code:\n");
	while(vm->pc < vector_size(buffer))
	{
		instruction_t* instr = vector_get(buffer, vm->pc);

		console("  %.2d: %s", vm->pc, op2str(instr->op));
		if(instr->v1)
		{
			console(", ");
			value_print(instr->v1);
		}
		if(instr->v2)
		{
			console(", ");
			value_print(instr->v2);
		}
		console("\n");
		vm->pc++;
	}
	vm->pc = 0;
}

// Processes a buffer instruction based on instruction / program counter (pc).
void vm_process(vm_t* vm, vector_t* buffer)
{
	instruction_t* instr = vector_get(buffer, vm->pc);

#ifndef NO_TRACE
	console("  %.2d (SP:%d, FP:%d): %s", vm->pc, vm->sp, vm->fp, op2str(instr->op));
	if(instr->v1)
	{
		console(", ");
		value_print(instr->v1);
	}
	if(instr->v2)
	{
		console(", ");
		value_print(instr->v2);
	}
	console(" => STACK [");
	for(int i = 0; i < vm->sp; i++)
	{
		if(vm->stack[i]) {
			value_print(vm->stack[i]);
			if(i < vm->sp-1) console(", ");
		}
	}
	console("]\n");
#endif

	switch(instr->op)
	{
		case OP_PUSH:
		{
			push(vm, value_copy(instr->v1));
			break;
		}
		case OP_POP:
		{
			pop(vm);
			break;
		}
		case OP_STORE:
		{
			// Local variable storage
			// HACK: increasing stack pointer
			// Otherwise:
			// If too many locals in last scope e.g. from 1 - 11
			// Then new scope overwrites old values e.g. fp at 5, overwrites 5- 11

			int offset = value_int(instr->v1);
			if(offset < 0)
			{
				vm->stack[vm->fp+offset] = pop(vm);
			}
			else
			{
				value_free(vm->locals[vm->fp+offset]);
				vm->locals[vm->fp+offset] = value_copy(pop(vm));
				if(vm->fp+offset >= vm->sp) vm->sp = vm->fp+offset;
			}
			break;
		}
		case OP_LOAD:
		{
			int offset = value_int(instr->v1);
			if(offset < 0)
			{
				push(vm, value_copy(vm->stack[vm->fp+offset]));
			}
			else
			{
				push(vm, value_copy(vm->locals[vm->fp+offset]));
			}
			break;
		}
		case OP_GSTORE:
		{
			int offset = value_int(instr->v1);
			value_free(vm->locals[offset]);
			vm->locals[offset] = value_copy(pop(vm));
			if(vm->fp+offset >= vm->sp) vm->sp = vm->fp+offset;
			break;
		}
		case OP_GLOAD:
		{
			int offset = value_int(instr->v1);
			value_t* v = vm->locals[offset];
			push(vm, value_copy(v));
			break;
		}
		case OP_LDARG0:
		{
			int args = value_int(vm->stack[vm->fp-3]);
			push(vm, value_copy(vm->stack[vm->fp-3-args]));
			break;
		}
		case OP_UPVAL:
		{
			int scopes = value_int(instr->v1);
			int offset = value_int(instr->v2);

			int fp = vm->fp;
			int sp = vm->sp;

			for(int i = 0; i < scopes; i++)
			{
				vm->fp = value_int(vm->stack[vm->fp - 2]);
			}
			vm->sp = vm->fp;

			value_t* v = 0;
			if(offset < 0)
				v = vm->stack[vm->fp+offset];
			else
				v = vm->locals[vm->fp+offset];

			vm->sp = sp;
			vm->fp = fp;
			push(vm, value_copy(v));
			break;
		}
		case OP_UPSTORE:
		{
			value_t* newVal = value_copy(pop(vm));

			int scopes = value_int(instr->v1);
			int offset = value_int(instr->v2);

			int fp = vm->fp;
			int sp = vm->sp;

			for(int i = 0; i < scopes; i++)
			{
				vm->fp = value_int(vm->stack[vm->fp - 2]);
			}
			vm->sp = vm->fp;

			if(offset < 0)
			{
				vm->stack[vm->fp+offset] = newVal;
			}
			else
			{
				value_free(vm->locals[vm->fp+offset]);
				vm->locals[vm->fp+offset] = newVal;
			}

			vm->sp = sp;
			vm->fp = fp;
			break;
		}
		case OP_SYSCALL:
		{
			char* name = value_string(instr->v1);
			value_t* v = value_copy(instr->v2);
			push(vm, v);

			int i = 0;
			while(system_methods[i].name)
			{
				if(!strcmp(system_methods[i].name, name))
				{
					system_methods[i].func(vm);
					break;
				}
				i++;
			}

			break;
		}
		case OP_INVOKE:
		{
			// Arguments already on the stack
			int address = value_int(instr->v1);
			size_t args = value_int(instr->v2);

			push(vm, value_new_int(args));
			push(vm, value_new_int(vm->fp));
			push(vm, value_new_int(vm->pc));

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
		case OP_RET:
		{
			// Returns to previous instruction pointer
			value_t* ret = pop(vm);
			vm->sp = vm->fp;

			vm->pc = value_int(pop(vm));
			vm->fp = value_int(pop(vm));

			vm->sp -= value_int(pop(vm));
			push(vm, value_copy(ret));
			break;
		}
		case OP_JMP:
		{
			vm->pc = value_int(instr->v1);
			return;
		}
		case OP_JMPF:
		{
			bool result = value_bool(pop(vm));
			if(!result)
			{
				vm->pc = value_int(instr->v1);
				return;
			}
			break;
		}
		case OP_ARR:
		{
			// Reverse list sorting
			size_t elsz = value_int(instr->v1);
			value_t** arr = malloc(sizeof(value_t*) * elsz);

			for(int i = elsz; i > 0; i--)
			{
				value_t* val = vm->stack[vm->sp - i];
				arr[elsz - i] = value_copy(val);
				vm->stack[vm->sp - i] = 0;
			}

			vm->sp -= elsz;
			value_t* ret = value_new_array(arr, elsz);
			push(vm, ret);
			break;
		}
		case OP_IADD:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) + value_int(v2));
			push(vm, v);
			break;
		}
		case OP_ISUB:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) - value_int(v2));
			push(vm, v);
			break;
		}
		case OP_IMUL:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) * value_int(v2));
			push(vm, v);
			break;
		}
		case OP_IDIV:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) / value_int(v2));
			push(vm, v);
			break;
		}
		case OP_MOD:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) % value_int(v2));
			push(vm, v);
			break;
		}
		case OP_BITL:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) << value_int(v2));
			push(vm, v);
			break;
		}
		case OP_BITR:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) >> value_int(v2));
			push(vm, v);
			break;
		}
		case OP_BITAND:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) & value_int(v2));
			push(vm, v);
			break;
		}
		case OP_BITOR:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) | value_int(v2));
			push(vm, v);
			break;
		}
		case OP_BITXOR:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) ^ value_int(v2));
			push(vm, v);
			break;
		}
		case OP_BITNOT:
		{
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(~value_int(v1));
			push(vm, v);
			break;
		}
		case OP_IMINUS:
		{
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(-value_int(v1));
			push(vm, v);
			break;
		}
		case OP_FADD:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_float(value_float(v1) + value_float(v2));
			push(vm, v);
			break;
		}
		case OP_FSUB:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_float(value_float(v1) - value_float(v2));
			push(vm, v);
			break;
		}
		case OP_FMUL:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_float(value_float(v1) * value_float(v2));
			push(vm, v);
			break;
		}
		case OP_FDIV:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_float(value_float(v1) / value_float(v2));
			push(vm, v);
			break;
		}
		case OP_FMINUS:
		{
			value_t* v1 = pop(vm);
			value_t* v = value_new_float(-value_float(v1));
			push(vm, v);
			break;
		}
		case OP_NOT:
		{
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(!value_bool(v1));
			push(vm, v);
			break;
		}
		case OP_BEQ:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_bool(v1) == value_bool(v2));
			push(vm, v);
			break;
		}
		case OP_IEQ:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_int(v1) == value_int(v2));
			push(vm, v);
			break;
		}
		case OP_FEQ:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_float(v1) == value_float(v2));
			push(vm, v);
			break;
		}
		case OP_CEQ:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_char(v1) == value_char(v2));
			push(vm, v);
			break;
		}
		case OP_BNE:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_bool(v1) != value_bool(v2));
			push(vm, v);
			break;
		}
		case OP_INE:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_int(v1) != value_int(v2));
			push(vm, v);
			break;
		}
		case OP_FNE:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_float(v1) != value_float(v2));
			push(vm, v);
			break;
		}
		case OP_CNE:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_char(v1) != value_char(v2));
			push(vm, v);
			break;
		}
		case OP_LT:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_number(v1) < value_number(v2));
			push(vm, v);
			break;
		}
		case OP_GT:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_number(v1) > value_number(v2));
			push(vm, v);
			break;
		}
		case OP_LE:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_number(v1) <= value_number(v2));
			push(vm, v);
			break;
		}
		case OP_GE:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_number(v1) >= value_number(v2));
			push(vm, v);
			break;
		}
		case OP_BAND:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_bool(v1) && value_bool(v2));
			push(vm, v);
			break;
		}
		case OP_BOR:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_bool(v1) || value_bool(v2));
			push(vm, v);
			break;
		}
		case OP_GETSUB:
		{
			// Stack:
			// | object |
			// | key 	|
			// | getsub |
			value_t* key = pop(vm);
			value_t* object = pop(vm);

			if(object->type == VALUE_STRING)
			{
				char* str = value_string(object);
				int idx = value_int(key);

				value_t* v = value_new_char(str[idx]);
				push(vm, v);
			}
			else
			{
				array_t* arr = value_array(object);
				int idx = value_int(key);
				value_t* v = value_copy(arr->data[idx]);
				push(vm, v);
			}
			break;
		}
		case OP_SETSUB:
		{
			// Stack:
			// | value	|
			// | object |
			// | key 	|
			// | setsub |
			value_t* key = pop(vm);
			value_t* object = pop(vm);
			value_t* val = pop(vm);

			if(object->type == VALUE_ARRAY)
			{
				value_t* newArr = value_copy(object);
				array_t* newArrData = value_array(newArr);

				int idx = value_int(key);
				value_free(newArrData->data[idx]);
				newArrData->data[idx] = value_copy(val);

				push(vm, newArr);
			}
			else if(object->type == VALUE_STRING)
			{
				char* data = value_string(object);
				int idx = value_int(key);
				data[idx] = value_char(val);

				value_t* v = value_new_string(data);
				push(vm, v);
			}
			break;
		}
		case OP_CLASS:
		{
			value_t* v = value_new_class();
			push(vm, v);
			break;
		}
		case OP_SETFIELD:
		{
			int index = value_int(instr->v1);

			value_t* val = value_copy(pop(vm));
			value_t* class = value_copy(pop(vm));

			class_t* cls = value_class(class);
			value_free(vector_get(cls->fields, index));
			vector_set(cls->fields, index, val);

			push(vm, class);
			break;
		}
		case OP_GETFIELD:
		{
			int index = value_int(instr->v1);
			value_t* class = value_copy(pop(vm));

			class_t* cls = value_class(class);
			value_t* val = value_copy(vector_get(cls->fields, index));

			push(vm, class);
			push(vm, val);
			break;
		}
		default: break;
	}

	vm->pc++;
}

// Executes a buffer / list of instructions
void vm_execute(vm_t* vm, vector_t* buffer)
{
	// Reset vm
	vm->sp = 0;
	vm->pc = 0;
	vm->fp = 0;
	memset(vm->stack, 0, STACK_SIZE * sizeof(value_t*));
	memset(vm->locals, 0, LOCALS_SIZE * sizeof(value_t*));

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

	while(vm->pc < vector_size(buffer))
	{
		vm_process(vm, buffer);
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
		value_free(vm->locals[i]);
		vm->locals[i] = 0;
	}
}

// Frees the memory used by the vm
void vm_free(vm_t* vm)
{
	free(vm);
}
