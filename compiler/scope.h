#ifndef scope_h
#define scope_h

#include <parser/ast.h>
#include <adt/hashmap.h>

// Symbol is a certain info for the compiler,
// can be a function, variable, class, etc.
// everything that has to be identified.
// variable => immutable / mutable + name + type
// function => name + returntype
// class    => name + functions + variables
typedef struct symbol_t
{
	ast_t* node;
	int address;
	datatype_t type;
	bool global;
	int used;
	struct symbol_t* ref;
} symbol_t;

// Scope: contains symbols
typedef struct scope_t
{
	ast_t* node;
	hashmap_t* symbols;
	hashmap_t* classes;
	struct scope_t* super;
	list_t* subscopes;
	int address;
} scope_t;

scope_t* scope_new();
void scope_free(scope_t* scope);

#endif