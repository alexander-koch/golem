
#include "libdef.h"
#include <vm/vm.h>
#include <time.h>
extern float strtof(const char* str, char** endptr);

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

int core_gen_signatures(list_t* toplevel) {
	signature_new();
	require_func();

	// print(T) -> void
	function_new("print", DATA_VOID, 1);
	function_add_param(0, DATA_GENERIC);
	function_upload(toplevel);

	// println(T) -> void
	function_new("println", DATA_VOID, 2);
	function_add_param(0, DATA_GENERIC);
	function_upload(toplevel);

	// getline() -> char[]
	function_new("getline", DATA_STRING, 3);
	function_upload(toplevel);

	// parseFloat(str:char[]) -> float
	function_new("parseFloat", DATA_FLOAT, 4);
	function_add_param(0, DATA_STRING);
	function_upload(toplevel);

	// break() -> void
	function_new("break", DATA_VOID, 5);
	function_upload(toplevel);

	// clock() -> void
	function_new("clock", DATA_FLOAT, 6);
	function_upload(toplevel);

	// sysarg(idx:int) -> char[]
	function_new("sysarg", DATA_STRING, 7);
	function_add_param(0, DATA_INT);
	function_upload(toplevel);

	return 0;
}
