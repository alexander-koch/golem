// Math standard library

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

int math_gen_signatures(list_t* toplevel) {
	signature_new();
	require_func();

	function_new("sin", DATA_FLOAT, INDEX(1));
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("cos", DATA_FLOAT, INDEX(2));
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("tan", DATA_FLOAT, INDEX(3));
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

    function_new("asin", DATA_FLOAT, INDEX(4));
    function_add_param(0, DATA_FLOAT);
    function_upload(toplevel);

    function_new("acos", DATA_FLOAT, INDEX(5));
    function_add_param(0, DATA_FLOAT);
    function_upload(toplevel);

    function_new("atan", DATA_FLOAT, INDEX(6));
    function_add_param(0, DATA_FLOAT);
    function_upload(toplevel);

    function_new("atan2", DATA_FLOAT, INDEX(7));
    function_add_param(0, DATA_FLOAT);
    function_add_param(0, DATA_FLOAT);
    function_upload(toplevel);

    function_new("sinh", DATA_FLOAT, INDEX(8));
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("cosh", DATA_FLOAT, INDEX(9));
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("tanh", DATA_FLOAT, INDEX(10));
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

    function_new("exp", DATA_FLOAT, INDEX(11));
    function_add_param(0, DATA_FLOAT);
    function_upload(toplevel);

    function_new("ln", DATA_FLOAT, INDEX(12));
    function_add_param(0, DATA_FLOAT);
    function_upload(toplevel);

    function_new("log", DATA_FLOAT, INDEX(13));
    function_add_param(0, DATA_FLOAT);
    function_upload(toplevel);

    function_new("pow", DATA_FLOAT, INDEX(14));
	function_add_param(0, DATA_FLOAT);
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("sqrt", DATA_FLOAT, INDEX(15));
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

	function_new("ceil", DATA_FLOAT, INDEX(16));
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

    function_new("floor", DATA_FLOAT, INDEX(17));
	function_add_param(0, DATA_FLOAT);
	function_upload(toplevel);

    function_new("abs", DATA_FLOAT, INDEX(18));
    function_add_param(0, DATA_FLOAT);
    function_upload(toplevel);

	function_new("prng", DATA_FLOAT, INDEX(19));
	function_upload(toplevel);

	return 0;
}
