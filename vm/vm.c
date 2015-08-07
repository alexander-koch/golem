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

extern int io_println(vm_t* vm);
extern int io_getline(vm_t* vm);
extern int str_strlen(vm_t* vm);

static FunctionDef system_methods[] = {
	{"println", io_println},	// io
	{"getline", io_getline},	// io
	{"length", str_strlen},	    // str
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

	//vm->libs = hashmap_new();
	//vm_open_libs(vm);
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
		mark(vm->stack[i]);
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

instruction_t* vm_peek(vm_t* vm, list_t* buffer)
{
	if(vm->pc >= list_size(buffer)) return 0;
	return list_get(buffer, vm->pc);
}

// Just prints out instruction codes
void vm_print_code(vm_t* vm, list_t* buffer)
{
	console("\nImmediate code:\n");
	while(vm->pc < list_size(buffer))
	{
		instruction_t* instr = list_get(buffer, vm->pc);

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
void vm_process(vm_t* vm, list_t* buffer)
{
	instruction_t* instr = list_get(buffer, vm->pc);

#ifndef NO_TRACE
	console("  %.2d: %s [", vm->pc, op2str(instr->op));
	for(int i = 0; i < vm->sp; i++)
	{
		value_print(vm->stack[i]);
		if(i < vm->sp-1) console(", ");
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
			int offset = value_int(instr->v1);
			value_free(vm->locals[vm->fp+offset]);
			vm->locals[vm->fp+offset] = value_copy(pop(vm));
			break;
		}
		case OP_LOAD:
		{
			int offset = value_int(instr->v1);

			// HACK: Using < 0 for locals
			// Still works, wondering if it really is a HACK.
			if(offset < 0)
			{
				value_t* v = vm->stack[vm->fp+offset];
				push(vm, value_copy(v));
			}
			else
			{
				value_t* v = vm->locals[vm->fp+offset];
				push(vm, value_copy(v));
			}
			break;
		}
		case OP_GSTORE:
		{
			int offset = value_int(instr->v1);
			value_free(vm->locals[offset]);
			vm->locals[offset] = value_copy(pop(vm));
			break;
		}
		case OP_GLOAD:
		{
			int offset = value_int(instr->v1);
			value_t* v = vm->locals[offset];
			push(vm, value_copy(v));
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
			// |FP				-2|
			// |PC				-1|
			// |...				 0|	<-- current position sp
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

			value_t* pc_ptr = pop(vm);
			value_t* fp_ptr = pop(vm);
			vm->pc = value_int(pc_ptr);
			vm->fp = value_int(fp_ptr);

			value_t* args_ptr = pop(vm);
			size_t args = value_int(args_ptr);

			vm->sp -= args;
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
			value_t* v = pop(vm);
			bool result = value_bool(v);
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
			size_t elsz = value_int(pop(vm));
			list_t* list = list_new();
			for(int i = elsz; i > 0; i--)
			{
				value_t* val = vm->stack[vm->sp - i];
				list_push(list, value_copy(val));
				vm->stack[vm->sp - i] = 0;
			}
			vm->sp -= elsz;

			value_t* ret = value_new_list(list);
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
		case OP_CONCAT:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			char* str = concat(value_string(v1), value_string(v2));
			value_t* v = value_new_string(str);
			push(vm, v);
			free(str);
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
			value_t* v = value_new_float(value_float(v1) == value_float(v2));
			push(vm, v);
			break;
		}
		case OP_STREQ:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(!strcmp(value_string(v1), value_string(v2)));
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
			value_t* v = value_new_float(value_float(v1) != value_float(v2));
			push(vm, v);
			break;
		}
		case OP_STRNE:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(strcmp(value_string(v1), value_string(v2)) != 0);
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

			if(key->type == VALUE_STRING)
			{
				char* str = value_string(object);
				int idx = value_int(key);
				char* nw = malloc(sizeof(char)*2);
				nw[0] = str[idx];
				nw[1] = '\0';

				value_t* v = value_new_string(nw);
				push(vm, v);
				free(nw);
			}
			else
			{
				list_t* list = value_list(object);
				int idx = value_int(key);
				value_t* v = value_copy(list_get(list, idx));
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

			if(object->type == VALUE_LIST)
			{
				list_iterator_t* iter = list_iterator_create(value_list(object));
				int i = 0;
				int idx = value_int(key);

				list_t* nl = list_new();
				while(!list_iterator_end(iter))
				{
					if(i == idx)
					{
						list_push(nl, value_copy(val));
					}
					else
					{
						list_push(nl, value_copy(list_iterator_next(iter)));
					}
					i++;
				}
				list_iterator_free(iter);

				value_t* v = value_new_list(nl);
				push(vm, v);
			}
			break;
		}
		default: break;
	}

	vm->pc++;
}

// Executes a buffer / list of instructions
void vm_execute(vm_t* vm, list_t* buffer)
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
#endif

	// Run
#ifndef NO_EXEC
	console("\nExecution:\n");
	clock_t begin = clock();
	while(vm->pc < list_size(buffer))
	{
		vm_process(vm, buffer);
	}

	clock_t end = clock();
	double elapsed = (double)(end - begin) / CLOCKS_PER_SEC;
	console("Elapsed time: %.4f (sec)\n", elapsed);
#endif

	// Move stack pointer to zero, -> clears all elements by gc
	vm->sp = 0;
	gc(vm);

	// Clear
	console("\n");
	for(int i = 0; i < LOCALS_SIZE; i++)
	{
		value_free(vm->locals[i]);
		vm->locals[i] = 0;
	}
}

// Frees the memory used by the vm
void vm_free(vm_t* vm)
{
	console("Freeing vm\n");
	free(vm);
}
