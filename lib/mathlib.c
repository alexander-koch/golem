
#include "libdef.h"
#include <math.h>
#include <core/util.h>
#include <vm/vm.h>

void math_abs(vm_t* vm) {
	double v1 = fabs(AS_NUM(vm_pop(vm)));
	vm_push(vm, NUM_VAL(v1));
}

void math_sin(vm_t* vm) {
	double v1 = sin(AS_NUM(vm_pop(vm)));
    vm_push(vm, NUM_VAL(v1));
}

void math_cos(vm_t* vm) {
	double v1 = cos(AS_NUM(vm_pop(vm)));
	vm_push(vm, NUM_VAL(v1));
}
void math_tan(vm_t* vm) {
	double v1 = tan(AS_NUM(vm_pop(vm)));
	vm_push(vm, NUM_VAL(v1));
}

void math_sqrt(vm_t* vm) {
	double v1 = sqrt(AS_NUM(vm_pop(vm)));
	vm_push(vm, NUM_VAL(v1));
}

void math_floor(vm_t* vm) {
	double v1 = floor(AS_NUM(vm_pop(vm)));
	vm_push(vm, NUM_VAL(v1));
}

void math_ceil(vm_t* vm) {
	double v1 = ceil(AS_NUM(vm_pop(vm)));
	vm_push(vm, NUM_VAL(v1));
}

void math_pow(vm_t* vm) {
	int v2 = AS_NUM(vm_pop(vm));
	int v1 = AS_NUM(vm_pop(vm));
	vm_push(vm, NUM_VAL(pow(v1, v2)));
}

void math_sinh(vm_t* vm) {
	double v1 = AS_NUM(vm_pop(vm));
	vm_push(vm, NUM_VAL(sinh(v1)));
}

void math_cosh(vm_t* vm) {
	double v1 = AS_NUM(vm_pop(vm));
	vm_push(vm, NUM_VAL(cosh(v1)));
}

void math_tanh(vm_t* vm) {
	double v1 = AS_NUM(vm_pop(vm));
	vm_push(vm, NUM_VAL(tanh(v1)));
}

void math_prng(vm_t* vm) {
	vm_push(vm, NUM_VAL(prng()));
}

int math_gen_signatures(list_t* toplevel) {
	signature_new();
	require_func();

	function_new("abs", DATA_FLOAT, 8);
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("sin", DATA_FLOAT, 9);
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("cos", DATA_FLOAT, 10);
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("tan", DATA_FLOAT, 11);
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("sqrt", DATA_FLOAT, 12);
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("floor", DATA_FLOAT, 13);
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("ceil", DATA_FLOAT, 14);
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("pow", DATA_FLOAT, 15);
	function_add_param(0, DATA_FLOAT);
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("sinh", DATA_FLOAT, 16);
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("cosh", DATA_FLOAT, 17);
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("tanh", DATA_FLOAT, 18);
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("prng", DATA_FLOAT, 19);
	function_upload(toplevel);

	return 0;
}
