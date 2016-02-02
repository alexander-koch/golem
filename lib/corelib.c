
#include "libdef.h"
#include <vm/vm.h>
#include <time.h>
extern float strtof(const char* str, char** endptr);

val_t core_print(vm_t* vm)
{
	val_print(pop(vm));
	return NULL_VAL;
}

val_t core_println(vm_t* vm)
{
	core_print(vm);
	putchar('\n');
	return NULL_VAL;
}

val_t core_getline(vm_t* vm)
{
	// Get input to buffer
	char buf[512];
	fgets(buf, sizeof(buf), stdin);
	return STRING_VAL(buf);
}

val_t core_parseFloat(vm_t* vm)
{
	char* str = AS_STRING(pop(vm));
	return NUM_VAL(strtof(str, 0));
}

val_t core_break(vm_t* vm)
{
	getchar();
	return NULL_VAL;
}

val_t core_clock(vm_t* vm)
{
	double clocktime = (double)clock() / (double)CLOCKS_PER_SEC;
	return NUM_VAL(clocktime);
}

val_t core_sysarg(vm_t* vm)
{
	int idx = AS_INT32(pop(vm));
	if(idx >= vm->argc || idx < 0) {
		return STRING_VAL("");
	}

	return STRING_VAL(vm->argv[idx]);
}

int core_gen_signatures(list_t* toplevel)
{
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
