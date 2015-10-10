
#include "libdef.h"
#include <vm/vm.h>
#include <adt/hashmap.h>

extern float strtof(const char* str, char** endptr);

GOLEM_API int core_print(vm_t* vm)
{
	//value_print(pop(vm));
	val_print(pop(vm));
	return 0;
}

GOLEM_API int core_println(vm_t* vm)
{
	int ret = core_print(vm);
	putchar('\n');
	return ret;
}

GOLEM_API int core_getline(vm_t* vm)
{
	// Get input to buffer
	char buf[512];
	fgets(buf, sizeof(buf), stdin);
	val_t val = STRING_VAL(buf);
	//value_t* val = value_new_string(buf);
	push(vm, val);
	return 0;
}

GOLEM_API int core_parseFloat(vm_t* vm)
{
	//char* str = value_string(pop(vm));
	//push(vm, value_new_float(strtof(str, 0)));
	return 0;
}

GOLEM_API int core_break(vm_t* vm)
{
	getchar();
	return 0;
}

GOLEM_API int lib_generate(void* hashptr)
{
	hashmap_t* hash = (hashmap_t*)hashptr;
	hashmap_set(hash, "print", core_print);
	hashmap_set(hash, "println", core_println);
	hashmap_set(hash, "getline", core_getline);
	hashmap_set(hash, "parseFloat", core_parseFloat);
	hashmap_set(hash, "break", core_break);
	return 0;
}

// @deprecated
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

	function_new("parseFloat", DATA_FLOAT);
	function_add_param(DATA_STRING);
	function_upload(toplevel);

	function_new("break", DATA_VOID);
	function_upload(toplevel);

	return 0;
}

GOLEM_API void* lib_signatures()
{
	list_t* toplevel = list_new();
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

	function_new("parseFloat", DATA_FLOAT);
	function_add_param(DATA_STRING);
	function_upload(toplevel);

	function_new("break", DATA_VOID);
	function_upload(toplevel);

	return toplevel;
}
