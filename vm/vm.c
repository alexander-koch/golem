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
			value_t* ident = stack_pop(vm->stack);
			value_t* val = stack_pop(vm->stack);

		/*	variable_t* var = malloc(sizeof(*var));
			var->val = val;

			value_t* value = value_new_object(var);*/
			hashmap_set(vm->fields, ident->v.str, val);
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
		case OP_CALL:
		{
			// PUSH NAME
			// PUSH arg0
			// PUSH arg1
			// PUSH argN
			// PUSH ARGCOUNT
			// CALL

			// Get arg count
			I64 argc = ((value_t*)stack_pop(vm->stack))->v.i;

			// Store arguments
			list_t* args = list_new();
			for(int i = 0; i < argc; i++)
			{
				list_push(args, stack_pop(vm->stack));
			}
			value_t* ident = stack_pop(vm->stack);

			if(!strcmp(ident->v.str, "println"))
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
