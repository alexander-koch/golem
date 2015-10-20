
#include "libdef.h"
#include <vm/vm.h>
#include <core/util.h>

GOLEM_API val_t io_readFile(vm_t* vm)
{
	size_t size = 0;
	char* filename = AS_STRING(pop(vm));

	FILE* file = fopen(filename, "rb");
	if(!file) return NULL_VAL;
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);
	char* source = (char*)malloc(size+1);
	if(!source) return NULL_VAL;

	fread(source, sizeof(char), size, file);
	source[size] = '\0';
	fclose(file);

	return STRING_NOCOPY_VAL(source);
}

GOLEM_API val_t io_writeFile(vm_t* vm)
{
	char* filename = AS_STRING(pop(vm));
	char* content = AS_STRING(pop(vm));
	char* mode = AS_STRING(pop(vm));

	FILE* fp = fopen(filename, mode);
	if(!fp) return NULL_VAL;

	fwrite(&content, sizeof(char), strlen(content), fp);
	fclose(fp);
	return NULL_VAL;
}

GOLEM_API int io_gen_signatures(list_t* toplevel)
{
	signature_new();
	require_func();

	// readFile(str:char[]) -> char[]
	function_new("readFile", DATA_STRING, 14);
	function_add_param(DATA_STRING);
	function_upload(toplevel);

	// writeFile(name:char[], content:char[], mode:char[]) -> void
	function_new("writeFile", DATA_VOID, 15);
	function_add_param(DATA_STRING);
	function_add_param(DATA_STRING);
	function_add_param(DATA_STRING);
	function_upload(toplevel);

	return 0;
}
