#include "vm.h"

vm_t* vm_new()
{
	vm_t* vm = malloc(sizeof(*vm));
	vm->stack = stack_new();
	vm->fields = hashmap_new();
	vm->pc = 0;
	vm->error = false;
	return vm;
}

void save_var(vm_t* vm, char* key, value_t* val, bool mutate)
{
	variable_t* var = 0;
	void* tmp = 0;
	if(hashmap_get(vm->fields, key, &tmp) != HMAP_MISSING)
	{
		var = (variable_t*)tmp;
		if(!var->mutate)
		{
			console("Can't modify the non-mutable variable '%s'\n", var->name);
			vm->error = true;
		}
		else
		{
			value_free(var->val);
			var->val = val;
			value_retain(var->val);
		}
	}
	else
	{
		var = malloc(sizeof(*var));
		var->name = key;
		var->mutate = mutate;
		var->val = val;
		value_retain(var->val);

		// Save in fields
		hashmap_set(vm->fields, key, var);
	}
}

/**
 *	Processes a buffer instruction based on program counter (pc).
 */
void vm_process(vm_t* vm, list_t* buffer)
{
	instruction_t* instr = list_get(buffer, vm->pc);

	// LOG OPCODES
#define LOG_OP
#ifdef LOG_OP
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
#endif

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
		case OP_STORE_FIELD:
		{
			char* key = value_string(instr->v1);
			bool mutate = value_bool(instr->v2);
			value_t* newVal = stack_pop(vm->stack);
			save_var(vm, key, newVal, mutate);
			break;
		}
		case OP_GET_FIELD:
		{
			void* value = 0;
			char* key = value_string(instr->v1);
			int err = hashmap_get(vm->fields, key, &value);

			variable_t* var = (variable_t*)value;
			if(var)
			{
				stack_push(vm->stack, var->val);
			}
			else
			{
				console("Could not find variable '%s'. Code %d\n", key, err);
				vm->error = true;
			}

			break;
		}
		case OP_INVOKE:
		{
			char* name = value_string(stack_pop(vm->stack));

			I64 args = value_int(instr->v1);
			list_t* values = list_new();
			for(int i = 0; i < args; i++)
			{
				value_t* val = stack_pop(vm->stack);
				list_push(values, val);
			}

			if(!strcmp(name, "println"))
			{
				for(int i = args; i > 0; i--)
				{
					value_t* v = list_get(values, i-1);
					value_print(v);
				}
				console("\n");
			}

			// TODO: implement
			list_free(values);
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
		default: break;
	}
	vm->pc++;
}

/**
 *	Executes a buffer
 */
void vm_execute(vm_t* vm, list_t* buffer)
{
	vm->pc = 0;
	vm->error = false;
	while(vm->pc < list_size(buffer) && !vm->error)
	{
		vm_process(vm, buffer);
	}
}

/**
 *	Frees the memory used by the vm
 */
void vm_free(vm_t* vm)
{
	console("Freeing vm\n");
	stack_free(vm->stack);

	hashmap_iterator_t* iter = hashmap_iterator_create(vm->fields);
	while(!hashmap_iterator_end(iter))
	{
		variable_t* var = hashmap_iterator_next(iter);
		if(var)
		{
			free(var->val);
			free(var);
		}
	}
	hashmap_iterator_free(iter);
	hashmap_free(vm->fields);

	free(vm);
}
