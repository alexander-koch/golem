#include "vm.h"

#define STACK_SIZE 256

vm_t* vm_new()
{
	vm_t* vm = malloc(sizeof(*vm));
	vm->error = false;
	vm->stack = 0;
	vm->pc = 0;
	vm->fp = 0;
	vm->numObjects = 0;
	vm->maxObjects = 0;
	return vm;
}

instruction_t* vm_peek(vm_t* vm, list_t* buffer)
{
	if(vm->pc >= list_size(buffer)) return 0;
	return list_get(buffer, vm->pc);
}

void vm_mark_all(vm_t* vm)
{
	for(int i = 0; i < vm->stack->top; i++)
	{
		value_mark(vm->stack->content[i]);
	}
}

void vm_sweep(vm_t* vm)
{

}

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

	console("  %.2d: %s [", vm->pc, op2str(instr->op));
	for(int i = 0; i < stack_size(vm->stack); i++)
	{
		value_print(vm->stack->content[i]);
		if(i < stack_size(vm->stack)-1) console(", ");
	}
	console("]\n");

	switch(instr->op)
	{
		case OP_PUSH:
		{
			stack_push(vm->stack, value_copy(instr->v1));
			break;
		}
		case OP_POP:
		{
			value_free((value_t*)stack_pop(vm->stack));
			break;
		}
		case OP_STORE:
		{
			value_t* v = stack_pop(vm->stack);
			int offset = value_int(instr->v1);
			value_free(vm->stack->content[vm->fp+offset]);
			vm->stack->content[vm->fp+offset] = v;
			break;
		}
		case OP_LOAD:
		{
			int offset = value_int(instr->v1);
			value_t* v = vm->stack->content[vm->fp+offset];
			stack_push(vm->stack, value_copy(v));
			break;
		}
		case OP_INVOKE:
		{
			char* name = value_string(instr->v1);
			I64 args = value_int(instr->v2);

			list_t* values = list_new();
			for(int i = 0; i < args; i++)
			{
				value_t* val = stack_pop(vm->stack);
				list_push(values, val);
			}

			// TODO: wrap native functions in new method
			if(!strcmp(name, "println"))
			{
				for(int i = args; i > 0; i--)
				{
					value_t* v = list_get(values, i-1);
					value_print(v);
				}
				console("\n");
			}

			// Else get method!, TODO!

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
		case OP_JMP:
		{
			vm->pc = value_int(instr->v1);
			return;
		}
		case OP_JMPF:
		{
			value_t* v = stack_pop(vm->stack);
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
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_int(value_int(v1) + value_int(v2));
			stack_push(vm->stack, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_ISUB:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_int(value_int(v1) - value_int(v2));
			stack_push(vm->stack, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_IMUL:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_int(value_int(v1) * value_int(v2));
			stack_push(vm->stack, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_IDIV:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_int(value_int(v1) / value_int(v2));
			stack_push(vm->stack, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_MOD:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_int(value_int(v1) % value_int(v2));
			stack_push(vm->stack, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITL:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_int(value_int(v1) << value_int(v2));
			stack_push(vm->stack, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITR:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_int(value_int(v1) >> value_int(v2));
			stack_push(vm->stack, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITAND:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_int(value_int(v1) & value_int(v2));
			stack_push(vm->stack, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITOR:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_int(value_int(v1) | value_int(v2));
			stack_push(vm->stack, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITXOR:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_int(value_int(v1) ^ value_int(v2));
			stack_push(vm->stack, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_CONCAT:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			char* str = concat(value_string(v1), value_string(v2));
			value_t* v = value_new_string(str);
			stack_push(vm->stack, v);
			free(str);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_IEQ:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_bool(value_int(v1) == value_int(v2));
			stack_push(vm->stack, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_STREQ:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_bool(!strcmp(value_string(v1), value_string(v2)));
			stack_push(vm->stack, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_INE:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_bool(value_int(v1) != value_int(v2));
			stack_push(vm->stack, v);
			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_ILT:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);
			value_t* v = value_new_bool(value_int(v1) < value_int(v2));
			stack_push(vm->stack, v);
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
	vm->stack = stack_new();
	vm->pc = 0;
	vm->fp = 100;
	vm->error = false;
	vm->maxObjects = 8;

	// Initialize stack
	stack_resize(vm->stack, STACK_SIZE);
	memset(vm->stack->content, 0, STACK_SIZE * sizeof(void*));

	// Print out bytecodes
	vm_print_code(vm, buffer);

	// Run
#ifndef NO_EXEC
	console("\nExecution:\n");
	while(vm->pc < list_size(buffer) && !vm->error)
	{
		vm_process(vm, buffer);
	}
#endif

	// Clear
	console("\n");
	while(stack_size(vm->stack) > 0)
	{
		value_t* v = stack_pop(vm->stack);
		value_free(v);
	}

	for(int i = vm->fp; i < vm->stack->size; i++)
	{
		value_free(vm->stack->content[i]);
	}

	stack_free(vm->stack);
}

// Frees the memory used by the vm
void vm_free(vm_t* vm)
{
	console("Freeing vm\n");
	free(vm);
}
