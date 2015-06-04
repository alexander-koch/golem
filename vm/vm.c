#include "vm.h"

vm_t* vm_new()
{
	vm_t* vm = malloc(sizeof(*vm));
	vm->stack = stack_new();
	vm->fields = hashmap_new();
	vm->pc = 0;
	return vm;
}

void vm_process(vm_t* vm, list_t* buffer)
{
	instruction_t* instr = list_get(buffer, vm->pc);

	// LOG OPCODES
	console("[%d] = (%s", vm->pc, op2str(instr->op));
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
	console(")\n");

	switch(instr->op)
	{
		case OP_PUSH_INT:
		{
			stack_push(vm->stack, instr->v1);
			break;
		}
		case OP_PUSH_FLOAT:
		{
			stack_push(vm->stack, instr->v1);
			break;
		}
		case OP_PUSH_STRING:
		{
			stack_push(vm->stack, instr->v1);
			break;
		}
		case OP_STORE:
		{
			value_t* val = stack_pop(vm->stack);
			char* ident = value_string(instr->v1);

			hashmap_set(vm->fields, ident, val);
			break;
		}
		case OP_MUT_STORE:
		{
			value_t* ident = stack_pop(vm->stack);
			value_t* val = stack_pop(vm->stack);
			hashmap_set(vm->fields, ident->v.str, val);
			break;
		}
		case OP_ADD:
		{
			value_t* v1 = stack_pop(vm->stack);
			value_t* v2 = stack_pop(vm->stack);

			if(v1->type == VALUE_INT && v2->type == VALUE_INT)
			{
				v1->v.i = v1->v.i + v2->v.i;
				stack_push(vm->stack, v1);
			}
			else if(v1->type == VALUE_FLOAT && v2->type == VALUE_FLOAT)
			{
				v1->v.f = v1->v.f + v2->v.f;
				stack_push(vm->stack, v1);
			}
			break;
		}
		case OP_LOAD:
		{
			void* val;
			hashmap_get(vm->fields, instr->v1->v.str, &val);
			value_t* ret = (value_t*)val;
			stack_push(vm->stack, ret);

			// TODO error handling if value not stored
			break;
		}
		case OP_ASSIGN:
		{
			// HACK!
			/*stack_pop(vm->stack);
			value_t* val = stack_pop(vm->stack);
			//hashmap_set(vm->fields, temp->v.str, val);*/
			break;
		}
		case OP_ARRAY:
		{
			I64 size = value_int(instr->v1);
			list_t* values = list_new();
			for(int i = 0; i < size; i++)
			{
				list_push(values, stack_pop(vm->stack));
			}

			// TODO: properly implement
			list_free(values);

			break;
		}
		case OP_CALL:
		{
			// PUSH arg0
			// PUSH arg1
			// PUSH argN
			// CALL "fnName" argument count

			// Get passed values (ident - argument count)
			char* ident = value_string(instr->v1);
			I64 argc = value_int(instr->v2);

			// Store arguments
			list_t* args = list_new();
			for(int i = 0; i < argc; i++)
			{
				list_push(args, stack_pop(vm->stack));
			}

			// TODO: valid function checks
			if(!strcmp(ident, "println"))
			{
				for(int i = argc-1; i >= 0; i--)
				{
					value_print(list_get(args, i));
				}
				console("\n");
			}

			list_free(args);
			break;
		}
		default: break;
	}
	vm->pc++;
}

void vm_execute(vm_t* vm, list_t* buffer)
{
	vm->pc = 0;
	while(vm->pc < list_size(buffer))
	{
		vm_process(vm, buffer);
	}
}

void vm_free(vm_t* vm)
{
	stack_free(vm->stack);
	hashmap_free(vm->fields);
	free(vm);
}
