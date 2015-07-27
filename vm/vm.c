#include "vm.h"

vm_t* vm_new()
{
	vm_t* vm = malloc(sizeof(*vm));
	vm->pc = 0;
	vm->fp = 0;
	vm->sp = 0;
	vm->numObjects = 0;
	vm->maxObjects = 0;
	return vm;
}

void push(vm_t* vm, value_t* val)
{
	value_free(vm->stack[vm->sp]);
	vm->stack[vm->sp] = val;
	vm->sp++;
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
	instruction_t* instr = vm_peek(vm, buffer);

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
			value_free((value_t*)pop(vm));
			break;
		}
		case OP_STORE:
		{
			// Local variable storage
			int offset = value_int(instr->v1);
			value_free(vm->locals[vm->fp+offset]);
			vm->locals[vm->fp+offset] = pop(vm);
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
		case OP_SYSCALL:
		{
			char* name = value_string(instr->v1);
			size_t args = value_int(instr->v2);

			list_t* values = list_new();
			for(int i = 0; i < args; i++)
			{
				value_t* val = pop(vm);
				list_push(values, val);
			}

			// TODO: wrap native functions in new method, handle different
			if(!strcmp(name, "println"))
			{
				for(int i = args; i > 0; i--)
				{
					value_t* v = list_get(values, i-1);
					value_print(v);
				}
				console("\n");
			}
			else if(!strcmp(name, "getline"))
			{
				char buf[1024];
				fgets(buf, sizeof(buf), stdin);

				value_t* val = value_new_string(buf);
				push(vm, val);
			}

			list_iterator_t* iter = list_iterator_create(values);
			while(!list_iterator_end(iter))
			{
				value_t* v = list_iterator_next(iter);
				value_free(v);
			}
			list_free(values);
			list_iterator_free(iter);
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

			// STACK_BOTTOM
			// ...
			// Arg0			-7
			// Arg1			-6
			// Arg2			-5
			// ...			-4
			// NUM_ARGS		-3
			// FP			-2
			// PC			-1
			// ...			 0	<-- current position sp
			// ...			+1
			// STACK_TOP

			vm->fp = vm->sp;
			vm->pc = address;
			return;
		}
		case OP_RET:
		{
			value_t* ret = pop(vm);
			vm->sp = vm->fp;

			value_t* pc_ptr = pop(vm);
			value_t* fp_ptr = pop(vm);
			vm->pc = value_int(pc_ptr);
			vm->fp = value_int(fp_ptr);
			value_free(pc_ptr);
			value_free(fp_ptr);

			value_t* args_ptr = pop(vm);
			size_t args = value_int(args_ptr);
			value_free(args_ptr);

			vm->sp -= args;
			push(vm, ret);
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
			value_free(v);
			if(!result)
			{
				vm->pc = value_int(instr->v1);
				return;
			}
			break;
		}
		case OP_IADD:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) + value_int(v2));
			push(vm, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_ISUB:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) - value_int(v2));
			push(vm, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_IMUL:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) * value_int(v2));
			push(vm, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_IDIV:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) / value_int(v2));
			push(vm, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_MOD:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) % value_int(v2));
			push(vm, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITL:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) << value_int(v2));
			push(vm, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITR:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) >> value_int(v2));
			push(vm, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITAND:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) & value_int(v2));
			push(vm, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITOR:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) | value_int(v2));
			push(vm, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITXOR:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_int(value_int(v1) ^ value_int(v2));
			push(vm, v);
			value_free(v1);
			value_free(v2);
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
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_IEQ:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_int(v1) == value_int(v2));
			push(vm, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_STREQ:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(!strcmp(value_string(v1), value_string(v2)));
			push(vm, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_INE:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_int(v1) != value_int(v2));
			push(vm, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_ILT:
		{
			value_t* v2 = pop(vm);
			value_t* v1 = pop(vm);
			value_t* v = value_new_bool(value_int(v1) < value_int(v2));
			push(vm, v);
			value_free(v1);
			value_free(v2);
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

	// Print out bytecodes
	vm_print_code(vm, buffer);

	// Run
#ifndef NO_EXEC
	console("\nExecution:\n");
	while(vm->pc < list_size(buffer))
	{
		vm_process(vm, buffer);
	}
#endif

	// Clear
	console("\n");
	for(int i = 0; i < STACK_SIZE; i++)
	{
		value_free(vm->stack[i]);
		vm->stack[i] = 0;
	}

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
