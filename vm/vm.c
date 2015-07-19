#include "vm.h"

vm_t* vm_new()
{
	vm_t* vm = malloc(sizeof(*vm));
	vm->stack = stack_new();
	vm->fields = hashmap_new();
	vm->functions = hashmap_new();
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

void save_var(vm_t* vm, char* key, value_t* val, bool mutate)
{
	variable_t* var = 0;
	void* tmp = 0;
	if(hashmap_get(vm->fields, key, &tmp) != HMAP_MISSING)
	{
		var = (variable_t*)tmp;
		if(!var->mutate)
		{
			vm_throw(vm, "Can't modify the non-mutable variable '%s'\n", var->name);
		}
		else
		{
			value_free(var->val);
			var->val = val;
		//	value_retain(var->val);
		}
	}
	else
	{
		var = malloc(sizeof(*var));
		var->name = key;
		var->mutate = mutate;
		var->val = val;
	//	value_retain(var->val);

		// Save in fields
		hashmap_set(vm->fields, key, var);
	}
}

void create_scope(vm_t* vm, list_t* buffer, char* name, I64 params)
{
	scope_t* func = 0;
	void* tmp = 0;
	if(hashmap_get(vm->functions, name, &tmp) != HMAP_MISSING)
	{
		vm_throw(vm, "Function redefinition\n");
	}
	else
	{
		func = malloc(sizeof(*func));
		func->name = name;
		func->args = params;
		func->pc = vm->pc+1;
		hashmap_set(vm->functions, name, func);

		instruction_t* ins = vm_peek(vm, buffer);
		while(ins)
		{
			ins = vm_peek(vm, buffer);
			if(ins->op == OP_POP_SCOPE)
			{
				func->ic = vm->pc - func->pc;
				break;
			}

			vm->pc++;
		}
	}
}

void run_scope(vm_t* vm, list_t* buffer, char* name, list_t* params)
{
	void* tmp = 0;
	if(hashmap_get(vm->functions, name, &tmp) != HMAP_MISSING)
	{
		scope_t* func = (scope_t*)tmp;
		if(list_size(params) != func->args)
		{
			vm_throw(vm, "Too few / many arguments for function '%s'\n", name);
		}
		else
		{
			U64 current = vm->pc;
			vm->pc = func->pc;
			for(int i = 0; i < func->ic; i++)
			{
				vm_process(vm, buffer);
			}
			vm->pc = current;
		}
	}
	else
	{
		vm_throw(vm, "Function with name '%s' is not defined\n", name);
	}
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
		case OP_PUSH_INT:
		{
			stack_push(vm->stack, value_copy(instr->v1));
			break;
		}
		case OP_PUSH_FLOAT:
		{
			stack_push(vm->stack, value_copy(instr->v1));
			break;
		}
		case OP_PUSH_STRING:
		{
			stack_push(vm->stack, value_copy(instr->v1));
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
				stack_push(vm->stack, value_copy(var->val));
			}
			else
			{
				console("Could not find variable '%s'. Code %d\n", key, err);
				vm->error = true;
			}

			break;
		}
		case OP_PUSH_SCOPE:
		{
			char* name = value_string(instr->v1);
			I64 params = value_int(instr->v2);
			create_scope(vm, buffer, name, params);
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
			else
			{
				run_scope(vm, buffer, name, values);
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
		case OP_ADD:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);

			if(v1->type == VALUE_INT && v2->type == VALUE_INT)
			{
				value_t* v = value_new_int(value_int(v1) + value_int(v2));
				stack_push(vm->stack, v);
			}
			else if(v1->type == VALUE_FLOAT && v2->type == VALUE_FLOAT)
			{
				value_t* v = value_new_float(value_float(v1) + value_float(v2));
				stack_push(vm->stack, v);
			}
			else if(v1->type == VALUE_STRING && v2->type == VALUE_STRING)
			{
				char* newstr = concat(value_string(v1), value_string(v2));
				value_t* v = value_new_string(newstr);
				stack_push(vm->stack, v);
				free(newstr);
			}
			else
			{
				vm_throw(vm, "No rule for adding objects of class '%s' and '%s'\n", value_classname(v1), value_classname(v2));
			}

			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_SUB:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);

			if(v1->type == VALUE_INT && v2->type == VALUE_INT)
			{
				value_t* v = value_new_int(value_int(v1) - value_int(v2));
				stack_push(vm->stack, v);
			}
			else if(v1->type == VALUE_FLOAT && v2->type == VALUE_FLOAT)
			{
				value_t* v = value_new_float(value_float(v1) - value_float(v2));
				stack_push(vm->stack, v);
			}
			else
			{
				vm_throw(vm, "No rule of subtraction for objects of class '%s' and '%s'\n", value_classname(v1), value_classname(v2));
			}

			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_MUL:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);

			if(v1->type == VALUE_INT && v2->type == VALUE_INT)
			{
				value_t* v = value_new_int(value_int(v1) * value_int(v2));
				stack_push(vm->stack, v);
			}
			else if(v1->type == VALUE_FLOAT && v2->type == VALUE_FLOAT)
			{
				value_t* v = value_new_float(value_float(v1) * value_float(v2));
				stack_push(vm->stack, v);
			}
			else
			{
				vm_throw(vm, "No rule for multipliying objects of class '%s' and '%s'\n", value_classname(v1), value_classname(v2));
			}

			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_DIV:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);

			if(v1->type == VALUE_INT && v2->type == VALUE_INT)
			{
				value_t* v = value_new_int(value_int(v1) / value_int(v2));
				stack_push(vm->stack, v);
			}
			else if(v1->type == VALUE_FLOAT && v2->type == VALUE_FLOAT)
			{
				value_t* v = value_new_float(value_float(v1) / value_float(v2));
				stack_push(vm->stack, v);
			}
			else
			{
				vm_throw(vm, "No rule for division of objects of class '%s' and '%s'\n", value_classname(v1), value_classname(v2));
			}

			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_MOD:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);

			if(v1->type == VALUE_INT && v2->type == VALUE_INT)
			{
				value_t* v = value_new_int(value_int(v1) % value_int(v2));
				stack_push(vm->stack, v);
			}
			else
			{
				vm_throw(vm, "Modulo is only applicable to integers\n");
			}

			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITL:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);

			if(v1->type == VALUE_INT && v2->type == VALUE_INT)
			{
				value_t* v = value_new_int(value_int(v1) << value_int(v2));
				stack_push(vm->stack, v);
			}
			else
			{
				vm_throw(vm, "Bitoperations are only applicable to integers\n");
			}

			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITR:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);

			if(v1->type == VALUE_INT && v2->type == VALUE_INT)
			{
				value_t* v = value_new_int(value_int(v1) >> value_int(v2));
				stack_push(vm->stack, v);
			}
			else
			{
				vm_throw(vm, "Bitoperations are only applicable to integers\n");
			}

			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITAND:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);

			if(v1->type == VALUE_INT && v2->type == VALUE_INT)
			{
				value_t* v = value_new_int(value_int(v1) & value_int(v2));
				stack_push(vm->stack, v);
			}
			else
			{
				vm_throw(vm, "Bitoperations are only applicable to integers\n");
			}

			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITOR:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);

			if(v1->type == VALUE_INT && v2->type == VALUE_INT)
			{
				value_t* v = value_new_int(value_int(v1) | value_int(v2));
				stack_push(vm->stack, v);
			}
			else
			{
				vm_throw(vm, "Bitoperations are only applicable to integers\n");
			}

			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_BITXOR:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);

			if(v1->type == VALUE_INT && v2->type == VALUE_INT)
			{
				value_t* v = value_new_int(value_int(v1) ^ value_int(v2));
				stack_push(vm->stack, v);
			}
			else
			{
				vm_throw(vm, "Bitoperations are only applicable to integers\n");
			}

			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_EQUAL:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);

			if(v1->type == VALUE_INT && v2->type == VALUE_INT)
			{
				value_t* v = value_new_bool(value_int(v1) == value_int(v2));
				stack_push(vm->stack, v);
			}
			else if(v1->type == VALUE_FLOAT && v2->type == VALUE_FLOAT)
			{
				value_t* v = value_new_bool(value_float(v1) == value_float(v2));
				stack_push(vm->stack, v);
			}
			else if(v1->type == VALUE_STRING && v2->type == VALUE_STRING)
			{
				char* s1 = value_string(v1);
				char* s2 = value_string(v2);
				value_t* v = value_new_bool(!strcmp(s1, s2));
				stack_push(vm->stack, v);
			}
			else
			{
				vm_throw(vm, "Cannot compare objects of classtypes '%s' and '%s'\n", value_classname(v1), value_classname(v2));
			}

			value_free(v1);
			value_free(v2);
			break;
		}
		case OP_LESS:
		{
			value_t* v2 = stack_pop(vm->stack);
			value_t* v1 = stack_pop(vm->stack);

			if(v1->type == VALUE_INT && v2->type == VALUE_INT)
			{
				value_t* v = value_new_bool(value_int(v1) < value_int(v2));
				stack_push(vm->stack, v);
			}
			else if(v1->type == VALUE_FLOAT && v2->type == VALUE_FLOAT)
			{
				value_t* v = value_new_bool(value_float(v1) < value_float(v2));
				stack_push(vm->stack, v);
			}
			else
			{
				vm_throw(vm, "Less operation cannot be applied for classes '%s' and '%s'\n", value_classname(v1), value_classname(v2));
			}

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

	// Free fields
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

	// Free scopes
	iter = hashmap_iterator_create(vm->functions);
	while(!hashmap_iterator_end(iter))
	{
	 	scope_t* scope = hashmap_iterator_next(iter);
		if(scope)
		{
			free(scope);
		}
	}
	hashmap_iterator_free(iter);

	// Free hashmaps and vm
	hashmap_free(vm->fields);
	hashmap_free(vm->functions);
	free(vm);
}
