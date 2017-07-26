// Math standard library
// Copyright (C) 2017 Alexander Koch

#include "libdef.h"
#include <math.h>
#include <core/util.h>
#include <vm/vm.h>

extern int corelib_fn_count;
#define INDEX(idx) (corelib_fn_count + idx)
int mathlib_fn_count = 19;

/**
 * function list:
 * 01 sin
 * 02 cos
 * 03 tan
 * 04 asin
 * 05 acos
 * 06 atan
 * 07 atan2
 * 08 sinh
 * 09 cosh
 * 10 tanh
 * 11 exp
 * 12 ln
 * 13 log
 * 14 pow
 * 15 sqrt
 * 16 ceil
 * 17 floor
 * 18 abs
 * 19 prng
 */

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

void math_asin(vm_t* vm) {
    double v1 = asin(AS_NUM(vm_pop(vm)));
    vm_push(vm, NUM_VAL(v1));
}

void math_acos(vm_t* vm) {
    double v1 = acos(AS_NUM(vm_pop(vm)));
    vm_push(vm, NUM_VAL(v1));
}

void math_atan(vm_t* vm) {
    double v1 = atan(AS_NUM(vm_pop(vm)));
    vm_push(vm, NUM_VAL(v1));
}

void math_atan2(vm_t* vm) {
    double v2 = AS_NUM(vm_pop(vm));
    double v1 = AS_NUM(vm_pop(vm));
    vm_push(vm, NUM_VAL(atan2(v1, v2)));
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

void math_exp(vm_t* vm) {
    double v1 = exp(AS_NUM(vm_pop(vm)));
    vm_push(vm, NUM_VAL(v1));
}

void math_ln(vm_t* vm) {
    double v1 = log(AS_NUM(vm_pop(vm)));
    vm_push(vm, NUM_VAL(v1));
}

void math_log(vm_t* vm) {
    double v1 = log10(AS_NUM(vm_pop(vm)));
    vm_push(vm, NUM_VAL(v1));
}

void math_pow(vm_t* vm) {
	int v2 = AS_NUM(vm_pop(vm));
	int v1 = AS_NUM(vm_pop(vm));
	vm_push(vm, NUM_VAL(pow(v1, v2)));
}

void math_sqrt(vm_t* vm) {
	double v1 = sqrt(AS_NUM(vm_pop(vm)));
	vm_push(vm, NUM_VAL(v1));
}

void math_ceil(vm_t* vm) {
	double v1 = ceil(AS_NUM(vm_pop(vm)));
	vm_push(vm, NUM_VAL(v1));
}

void math_floor(vm_t* vm) {
	double v1 = floor(AS_NUM(vm_pop(vm)));
	vm_push(vm, NUM_VAL(v1));
}

void math_abs(vm_t* vm) {
	double v1 = fabs(AS_NUM(vm_pop(vm)));
	vm_push(vm, NUM_VAL(v1));
}

void math_prng(vm_t* vm) {
	vm_push(vm, NUM_VAL(prng()));
}

int math_gen_signatures(context_t* context, list_t* toplevel) {
	signature_new();
	require_func();

    datatype_t* float_type = context_get(context, "float");

	function_new("sin", float_type, INDEX(1));
	function_add_param(NULL, float_type);
	function_upload(toplevel);

	function_new("cos", float_type, INDEX(2));
	function_add_param(NULL, float_type);
	function_upload(toplevel);

	function_new("tan", float_type, INDEX(3));
	function_add_param(NULL, float_type);
	function_upload(toplevel);

    function_new("asin", float_type, INDEX(4));
    function_add_param(NULL, float_type);
    function_upload(toplevel);

    function_new("acos", float_type, INDEX(5));
    function_add_param(NULL, float_type);
    function_upload(toplevel);

    function_new("atan", float_type, INDEX(6));
    function_add_param(NULL, float_type);
    function_upload(toplevel);

    function_new("atan2", float_type, INDEX(7));
    function_add_param(NULL, float_type);
    function_add_param(NULL, float_type);
    function_upload(toplevel);

    function_new("sinh", float_type, INDEX(8));
	function_add_param(NULL, float_type);
	function_upload(toplevel);

	function_new("cosh", float_type, INDEX(9));
	function_add_param(NULL, float_type);
	function_upload(toplevel);

	function_new("tanh", float_type, INDEX(10));
	function_add_param(NULL, float_type);
	function_upload(toplevel);

    function_new("exp", float_type, INDEX(11));
    function_add_param(NULL, float_type);
    function_upload(toplevel);

    function_new("ln", float_type, INDEX(12));
    function_add_param(NULL, float_type);
    function_upload(toplevel);

    function_new("log", float_type, INDEX(13));
    function_add_param(NULL, float_type);
    function_upload(toplevel);

    function_new("pow", float_type, INDEX(14));
	function_add_param(NULL, float_type);
	function_add_param(NULL, float_type);
	function_upload(toplevel);

	function_new("sqrt", float_type, INDEX(15));
	function_add_param(NULL, float_type);
	function_upload(toplevel);

	function_new("ceil", float_type, INDEX(16));
	function_add_param(NULL, float_type);
	function_upload(toplevel);

    function_new("floor", float_type, INDEX(17));
	function_add_param(NULL, float_type);
	function_upload(toplevel);

    function_new("abs", float_type, INDEX(18));
    function_add_param(NULL, float_type);
    function_upload(toplevel);

	function_new("prng", float_type, INDEX(19));
	function_upload(toplevel);

	return 0;
}
