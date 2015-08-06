
#define mathlib_c
#include <math.h>
#include <vm/vm.h>
#include <adt/hashmap.h>

int math_abs(vm_t* vm)
{
	pop(vm);
	value_t* v = value_new_float(fabs(value_number(pop(vm))));
	push(vm, v);
	return 0;
}

int math_sin(vm_t* vm)
{
	pop(vm);
	value_t* v = value_new_float(sin(value_number(pop(vm))));
	push(vm, v);
	return 0;
}

int math_cos(vm_t* vm)
{
	pop(vm);
	value_t* v = value_new_float(cos(value_number(pop(vm))));
	push(vm, v);
	return 0;
}

static const FunctionDef mathlib[] = {
	{"abs", math_abs},
	{"sin", math_sin},
	{"cos", math_cos},
	{0, 0}
};

int open_mathlib(hashmap_t* symbols)
{
	const FunctionDef* lib;
	for(lib = mathlib; lib->func; lib++)  {
		hashmap_set(symbols, (char*)lib->name, lib->func);
	}
	return 0;
}
