
#define mathlib_c
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

GOLEM_API int math_sqrt(vm_t* vm)
{
	pop(vm);
	value_t* v = value_new_float(sqrt(value_number(pop(vm))));
	push(vm, v);
	return 0;
}

static const FunctionDef mathlib[] = {
	{"abs", math_abs},
	{"sin", math_sin},
	{"cos", math_cos},
	{"sqrt", math_sqrt},
	{0, 0}
};

GOLEM_API int math_gen_signatures(list_t* toplevel)
{
	SIGNATURE_BEGIN()

	FUNCTION_NEW("abs", DATA_FLOAT)
	ADD_PARAM(DATA_FLOAT)
	FUNCTION_PUSH(toplevel)

	FUNCTION_NEW("sin", DATA_FLOAT)
	ADD_PARAM(DATA_FLOAT)
	FUNCTION_PUSH(toplevel)

	FUNCTION_NEW("cos", DATA_FLOAT)
	ADD_PARAM(DATA_FLOAT)
	FUNCTION_PUSH(toplevel)

	FUNCTION_NEW("sqrt", DATA_FLOAT)
	ADD_PARAM(DATA_FLOAT)
	FUNCTION_PUSH(toplevel)
	return 4;
}
