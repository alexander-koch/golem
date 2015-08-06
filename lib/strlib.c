
#define gstrlib_c

#include <string.h>
#include <vm/vm.h>
#include <adt/hashmap.h>

int str_strlen(vm_t* vm)
{
	pop(vm);
	value_t* str = pop(vm);

	// Don't mind the null-terminator
	size_t len = strlen(value_string(str))-1;
	value_t* v = value_new_int(len);
	push(vm, v);
	return 0;
}

static const FunctionDef strlib[] = {
	{"strlen", str_strlen},
	{0, 0}
};

int open_strlib(hashmap_t* symbols)
{
	const FunctionDef* lib;
	for(lib = strlib; lib->func; lib++)  {
		hashmap_set(symbols, (char*)lib->name, lib->func);
	}
	return 0;
}
