#include "vm.h"

vm_t* vm_new()
{
	vm_t* vm = malloc(sizeof(*vm));
	vm->stack = stack_new();
	vm->pc = 0;
	vm->error = false;
	return vm;
}

void vm_throw(vm_t* vm, const char* format, ...)
{
	vm->error = true;
    //location_t loc = opt->node->location;
    //fprintf(stdout, "[line %d, column %d] (Semantic): ", loc.line, loc.column);
	// TODO: get line / column position for OP_CODE

	fprintf(stdout, "(VM): ");
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
    fprintf(stdout, "\n");
}

instruction_t* vm_peek(vm_t* vm, list_t* buffer)
{
	if(vm->pc >= list_size(buffer)) return 0;
	return list_get(buffer, vm->pc);
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

/**
 *	Processes a buffer instruction based on program counter (pc).
 */
void vm_process(vm_t* vm, list_t* buffer)
{
	instruction_t* instr = list_get(buffer, vm->pc);

	switch(instr->op)
	{
		case OP_ICONST:
		{
			stack_push(vm->stack, value_copy(instr->v1));
			break;
		}
		case OP_FCONST:
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
			// char* key = value_string(instr->v1);
			// bool mutate = value_bool(instr->v2);
			// value_t* v = stack_pop(vm->stack);
			// save_var(vm, key, v, mutate);
			break;
		}
		case OP_LOAD:
		{
			// void* value = 0;
			// char* key = value_string(instr->v1);
			// int err = hashmap_get(vm->fields, key, &value);
			//
			// variable_t* var = (variable_t*)value;
			// if(var)
			// {
			// 	stack_push(vm->stack, value_copy(var->val));
			// }
			// else
			// {
			// 	console("Could not find variable '%s'. Code %d\n", key, err);
			// 	vm->error = true;
			// }

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

			// Else search!

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
		default:
		{
			vm_throw(vm, "Unknown op-code\n");
			break;
		}
	}

//	console("  %.2d (stack %d)\n", vm->pc, stack_size(vm->stack));
	vm->pc++;
}

/**
 *	Executes a buffer
 */
void vm_execute(vm_t* vm, list_t* buffer)
{
	vm->pc = 0;
	vm->error = false;
	vm_print_code(vm, buffer);

	console("\nExecution:\n");
	while(vm->pc < list_size(buffer) && !vm->error)
	{
		vm_process(vm, buffer);
	}

	console("\n");
	while(stack_size(vm->stack) > 0)
	{
		value_t* v = stack_pop(vm->stack);
		value_free(v);
	}
}

/**
 *	Frees the memory used by the vm
 */
void vm_free(vm_t* vm)
{
	console("Freeing vm\n");
	stack_free(vm->stack);
	free(vm);
}
