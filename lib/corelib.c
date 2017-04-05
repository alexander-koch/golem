// Core standard library

#include "libdef.h"
#include <vm/vm.h>
#include <time.h>
extern float strtof(const char* str, char** endptr);

int corelib_fn_count = 7;

/**
 * function list:
 * 01 print
 * 02 println
 * 03 getline
 * 04 parseFloat
 * 05 break
 * 06 clock
 * 07 sysarg
 */

void core_print(vm_t* vm) {
	val_print(vm_pop(vm));
	vm_push(vm, NULL_VAL);
}

void core_println(vm_t* vm) {
	val_print(vm_pop(vm));
	putchar('\n');
    vm_push(vm, NULL_VAL);
}

void core_getline(vm_t* vm) {
	// Get input to buffer
	char buf[512];
	fgets(buf, sizeof(buf), stdin);
    vm_register(vm, STRING_VAL(buf));
}

void core_parseFloat(vm_t* vm) {
	char* str = AS_STRING(vm_pop(vm));
	vm_push(vm, NUM_VAL(strtof(str, 0)));
}

void core_break(vm_t* vm) {
	getchar();
	vm_push(vm, NULL_VAL);
}

void core_clock(vm_t* vm) {
	double clocktime = (double)clock() / (double)CLOCKS_PER_SEC;
    vm_push(vm, NUM_VAL(clocktime));
}

void core_sysarg(vm_t* vm) {
	int idx = AS_INT32(vm_pop(vm));
    val_t val;
	if(idx >= vm->argc || idx < 0) {
	    val = STRING_VAL("");
	} else {
        val = STRING_VAL(vm->argv[idx]);
    }
    vm_register(vm, val);
}

int core_gen_signatures(context_t* context, list_t* toplevel) {
	signature_new();
	require_func();

    datatype_t* void_type = context_get(context, "void");
    datatype_t* float_type = context_get(context, "float");
    datatype_t* int_type = context_get(context, "int");
    datatype_t* generic_type = context_get(context, "generic");
    datatype_t* string_type = context_get(context, "str");

	// print(T) -> void
	function_new("print", void_type, 1);
	function_add_param(NULL, generic_type);
	function_upload(toplevel);

	// println(T) -> void
	function_new("println", void_type, 2);
	function_add_param(NULL, generic_type);
	function_upload(toplevel);

	// getline() -> char[]
	function_new("getline", string_type, 3);
	function_upload(toplevel);

	// parseFloat(str:char[]) -> float
	function_new("parseFloat", float_type, 4);
	function_add_param(NULL, string_type);
	function_upload(toplevel);

	// break() -> void
	function_new("break", void_type, 5);
	function_upload(toplevel);

	// clock() -> void
	function_new("clock", float_type, 6);
	function_upload(toplevel);

	// sysarg(idx:int) -> char[]
	function_new("sysarg", string_type, 7);
	function_add_param(NULL, int_type);
	function_upload(toplevel);

	return 0;
}
