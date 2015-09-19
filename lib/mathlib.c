
#include "libdef.h"
#include <math.h>
#include <vm/vm.h>

GOLEM_API int math_abs(vm_t* vm)
{
	pop(vm);
	value_t* v = value_new_float(fabs(value_number(pop(vm))));
	push(vm, v);
	return 0;
}

GOLEM_API int math_sin(vm_t* vm)
{
	pop(vm);
	value_t* v = value_new_float(sin(value_number(pop(vm))));
	push(vm, v);
	return 0;
}

GOLEM_API int math_cos(vm_t* vm)
{
	pop(vm);
	value_t* v = value_new_float(cos(value_number(pop(vm))));
	push(vm, v);
	return 0;
}

GOLEM_API int math_tan(vm_t* vm)
{
	pop(vm);
	value_t* v = value_new_float(tan(value_number(pop(vm))));
	push(vm, v);
	return 0;
}

GOLEM_API int math_sqrt(vm_t* vm)
{
	pop(vm);
	value_t* v = value_new_float(sqrt(value_number(pop(vm))));
	push(vm, v);
	return 0;
}

GOLEM_API int lib_generate(void* hashptr)
{
	hashmap_t* hash = (hashmap_t*)hashptr;
	hashmap_set(hash, "abs", math_abs);
	hashmap_set(hash, "sin", math_sin);
	hashmap_set(hash, "cos", math_cos);
	hashmap_set(hash, "tan", math_tan);
	return 0;
}

GOLEM_API void* lib_signatures()
{
	list_t* toplevel = list_new();
	signature_new();
	require_func();

	function_new("abs", DATA_FLOAT);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	function_new("sin", DATA_FLOAT);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	function_new("cos", DATA_FLOAT);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	function_new("tan", DATA_FLOAT);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	function_new("sqrt", DATA_FLOAT);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	return 0;
}
