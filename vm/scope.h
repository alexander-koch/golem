#ifndef scope_h
#define scope_h

#include <core/mem.h>
#include <vm/value.h>
#include <adt/hashmap.h>

typedef struct scope_t
{
	hashmap_t* variables;
	struct scope_t* subscopes;
} scope_t;

scope_t* scope_new();
void scope_add_var(scope_t* scope, char* key, value_t* value);
value_t* scope_get_var(scope_t* scope, char* key);
void scope_free(scope_t* scope);

#endif
