#include "scope.h"

scope_t* scope_new()
{
	scope_t* scope = malloc(sizeof(*scope));
	scope->variables = hashmap_new();
	return scope;
}

void scope_add_var(scope_t* scope, char* key, value_t* value)
{
	hashmap_set(scope->variables, key, value);
}

value_t* scope_get_var(scope_t* scope, char* key)
{
	void* value = 0;
	if(hashmap_get(scope->variables, key, value) != HMAP_OK)
	{
		return 0;
	}
	value_t* ret = (value_t*)value;
	return ret;
}

void scope_free(scope_t* scope)
{
	if(hashmap_length(scope->variables) > 0)
	{
		for (int i = 0; i < scope->variables->table_size; i++)
		{
			if(scope->variables->data[i].use)
			{
				value_t* val = (scope->variables->data[i].data);
				value_free(val);
			}
		}
	}

	hashmap_free(scope->variables);
	free(scope);
}
