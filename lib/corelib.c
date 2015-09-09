
#define core_c
#include "libdef.h"
#include <vm/vm.h>
#include <adt/hashmap.h>

extern float strtof(const char* str, char** endptr);

int core_print(vm_t* vm)
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

int core_println(vm_t* vm)
{
	int ret = core_print(vm);
	printf("\n");
	return ret;
}

int core_getline(vm_t* vm)
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

int core_f2i(vm_t* vm)
{
	pop(vm);
	float f = value_float(pop(vm));
	push(vm, value_new_int(f));
	return 0;
}

int core_i2f(vm_t* vm)
{
	pop(vm);
	int i = value_int(pop(vm));
	push(vm, value_new_float(i));
	return 0;
}

int core_c2i(vm_t* vm)
{
	pop(vm);
	char c = value_char(pop(vm));
	push(vm, value_new_int(c));
	return 0;
}

int core_parseFloat(vm_t* vm)
{
	pop(vm);
	char* str = value_string(pop(vm));
	push(vm, value_new_float(strtof(str, 0)));
	return 0;
}

static const FunctionDef core[] = {
	{"print", core_print},
	{"println", core_println},
	{"getline", core_getline},
	{"f2i", core_f2i},
	{"i2f", core_i2f},
	{"c2i", core_c2i},
	{"parseFloat", core_parseFloat},
	{0, 0}
};

// int open_core(hashmap_t* symbols)
// {
// 	const FunctionDef* lib;
// 	for(lib = iolib; lib->func; lib++)  {
// 		hashmap_set(symbols, (char*)lib->name, lib->func);
// 	}
// 	return 0;
// }

int core_gen_signatures(list_t* toplevel)
{
	signature_new();
	require_func();

	function_new("getline", DATA_STRING);
	function_upload(toplevel);

	function_new("print", DATA_VOID);
	function_add_param(DATA_GENERIC);
	function_upload(toplevel);

	function_new("println", DATA_VOID);
	function_add_param(DATA_GENERIC);
	function_upload(toplevel);

	function_new("f2i", DATA_INT);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	function_new("i2f", DATA_INT);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	function_new("c2i", DATA_INT);
	function_add_param(DATA_CHAR);
	function_upload(toplevel);

	function_new("parseFloat", DATA_FLOAT);
	function_add_param(DATA_STRING);
	function_upload(toplevel);

	return 0;
}
