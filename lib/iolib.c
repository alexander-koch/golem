
#define iolib_c
#include <vm/vm.h>
#include <adt/hashmap.h>

int io_println(vm_t* vm)
{
	size_t argc = value_int(pop(vm));

	// Order values
	list_t* values = list_new();
	for(int i = 0; i < argc; i++)
	{
		value_t* val = pop(vm);
		list_push(values, val);
	}

	// Print values
	for(int i = argc; i > 0; i--)
	{
		value_t* v = list_get(values, i-1);
		value_print(v);
	}
	console("\n");
	list_free(values);
	return 0;
}

int io_getline(vm_t* vm)
{
	// Discard argument count
	pop(vm);

	// Get input to buffer
	char buf[512];
	fgets(buf, sizeof(buf), stdin);
	value_t* val = value_new_string(buf);
	push(vm, val);
	return 0;
}

static const FunctionDef iolib[] = {
	{"println", io_println},
	{"getline", io_getline},
	{0, 0}
};

int open_iolib(hashmap_t* symbols)
{
	const FunctionDef* lib;
	for(lib = iolib; lib->func; lib++)  {
		hashmap_set(symbols, (char*)lib->name, lib->func);
	}
	return 0;
}

void io_gen_signatures(list_t* toplevel)
{
	location_t loc = {0, 0};
	ast_t* getln = ast_class_create(AST_DECLFUNC, loc);
	getln->funcdecl.name = strdup("getline");
	getln->funcdecl.impl.formals = list_new();
	getln->funcdecl.impl.body = 0;
	getln->funcdecl.rettype = DATA_STRING;
	getln->funcdecl.external = true;
	list_push(toplevel, getln);

	ast_t* prntln = ast_class_create(AST_DECLFUNC, loc);
	prntln->funcdecl.name = strdup("println");
	prntln->funcdecl.impl.formals = list_new();
	prntln->funcdecl.impl.body = 0;
	prntln->funcdecl.rettype = DATA_NULL;
	prntln->funcdecl.external = true;
	param_t* param = malloc(sizeof(*param));
	param->name = 0;
	param->type = DATA_VARARGS;
	list_push(prntln->funcdecl.impl.formals, param);
	list_push(toplevel, prntln);
}
