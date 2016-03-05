/**
 * scope.h
 * @author Alexander Koch
 * @desc Scope definition and helper functions.
 * Used to store symbols and bind variables to a certain lifetime.
 */

#ifndef scope_h
#define scope_h

#include <parser/ast.h>
#include <adt/hashmap.h>

/**
 * A symbol is an info for the compiler.
 * It can be a function, a variable, a class, etc.
 * Based on the symbol the compiler can reload it at any time
 * and check it's definition.
 *
 * Examples:
 * Variable => immutable / mutable + name + type
 * Function => name + returntype
 * Class    => name + functions + attributes / variables
 */
typedef struct symbol_t {
	ast_t* node;
	int address;
	datatype_t type;
	bool global;
	bool isClassParam;
	struct symbol_t* ref;
	int arraySize;
} symbol_t;

// Scope: contains symbols
typedef struct scope_t {
	ast_t* node;
	hashmap_t* symbols;
	hashmap_t* classes;
	struct scope_t* super;
	list_t* subscopes;
	int address;
	int flag;
	bool virtual;
} scope_t;

scope_t* scope_new();
void scope_free(scope_t* scope);
bool scope_requests(scope_t* scope, annotation_t ann);
void scope_unflag(scope_t* scope);

#endif
