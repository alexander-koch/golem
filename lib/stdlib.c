
#define stdlib_c
#include "libdef.h"
#include <vm/vm.h>
#include <adt/hashmap.h>

int stdlib_print(vm_t* vm)
{
	size_t argc = value_int(pop(vm));

	// Order values
	list_t* values = list_new();
	for(int i = 0; i < argc; i++)
	{
		value_t* val = pop(vm);
		list_push(values, val);
	}

	// Print values
	for(int i = argc; i > 0; i--)
	{
		value_t* v = list_get(values, i-1);
		value_print(v);
	}
	list_free(values);
	return 0;
}

int stdlib_println(vm_t* vm)
{
	int ret = stdlib_print(vm);
	printf("\n");
	return ret;
}

int stdlib_getline(vm_t* vm)
{
	// Discard argument count
	pop(vm);

	// Get input to buffer
	char buf[512];
	fgets(buf, sizeof(buf), stdin);
	value_t* val = value_new_string(buf);
	push(vm, val);
	return 0;
}

int stdlib_f2i(vm_t* vm)
{
	pop(vm);
	value_t* fl = pop(vm);
	value_t* val = value_new_int((int)value_float(fl));
	push(vm, val);
	return 0;
}

int stdlib_i2f(vm_t* vm)
{
	pop(vm);
	value_t* i = pop(vm);
	value_t* val = value_new_float((float)value_int(i));
	push(vm, val);
	return 0;
}

static const FunctionDef stdlib[] = {
	{"print", stdlib_print},
	{"println", stdlib_println},
	{"getline", stdlib_getline},
	{"f2i", stdlib_f2i},
	{"i2f", stdlib_i2f},
	{0, 0}
};

// int open_stdlib(hashmap_t* symbols)
// {
// 	const FunctionDef* lib;
// 	for(lib = iolib; lib->func; lib++)  {
// 		hashmap_set(symbols, (char*)lib->name, lib->func);
// 	}
// 	return 0;
// }

void stdlib_gen_signatures(list_t* toplevel)
{
	SIGNATURE_BEGIN()

	FUNCTION_NEW("getline", DATA_STRING)
	FUNCTION_PUSH(toplevel)

	FUNCTION_NEW("print", DATA_VOID)
	ADD_PARAM(DATA_VARARGS)
	FUNCTION_PUSH(toplevel)

	FUNCTION_NEW("println", DATA_VOID)
	ADD_PARAM(DATA_VARARGS)
	FUNCTION_PUSH(toplevel)

	FUNCTION_NEW("f2i", DATA_INT)
	ADD_PARAM(DATA_FLOAT)
	FUNCTION_PUSH(toplevel)

	FUNCTION_NEW("i2f", DATA_FLOAT)
	ADD_PARAM(DATA_INT)
	FUNCTION_PUSH(toplevel)
}
