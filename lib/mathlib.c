
#include "libdef.h"
#include <math.h>
#include <vm/vm.h>

GOLEM_API val_t math_abs(vm_t* vm)
{
	double v1 = fabs(AS_NUM(pop(vm)));
	return NUM_VAL(v1);
}

GOLEM_API val_t math_sin(vm_t* vm)
{
	double v1 = sin(AS_NUM(pop(vm)));
	return NUM_VAL(v1);
}

GOLEM_API val_t math_cos(vm_t* vm)
{
	double v1 = cos(AS_NUM(pop(vm)));
	return NUM_VAL(v1);
}

GOLEM_API val_t math_tan(vm_t* vm)
{
	double v1 = tan(AS_NUM(pop(vm)));
	return NUM_VAL(v1);
}

GOLEM_API val_t math_sqrt(vm_t* vm)
{
	double v1 = sqrt(AS_NUM(pop(vm)));
	return NUM_VAL(v1);
}

GOLEM_API val_t math_floor(vm_t* vm)
{
	double v1 = floor(AS_NUM(pop(vm)));
	return NUM_VAL(v1);
}

GOLEM_API val_t math_ceil(vm_t* vm)
{
	double v1 = ceil(AS_NUM(pop(vm)));
	return NUM_VAL(v1);
}

GOLEM_API val_t math_pow(vm_t* vm)
{
	int v2 = AS_NUM(pop(vm));
	int v1 = AS_NUM(pop(vm));
	return NUM_VAL(pow(v1, v2));
}

GOLEM_API int math_gen_signatures(list_t* toplevel)
{
	signature_new();
	require_func();

	function_new("abs", DATA_FLOAT, 6);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	function_new("sin", DATA_FLOAT, 7);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	function_new("cos", DATA_FLOAT, 8);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	function_new("tan", DATA_FLOAT, 9);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	function_new("sqrt", DATA_FLOAT, 10);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	function_new("floor", DATA_FLOAT, 11);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	function_new("ceil", DATA_FLOAT, 12);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	function_new("pow", DATA_FLOAT, 13);
	function_add_param(DATA_FLOAT);
	function_add_param(DATA_FLOAT);
	function_upload(toplevel);

	return 0;
}
