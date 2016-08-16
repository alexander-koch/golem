
#include "libdef.h"
#include <vm/vm.h>
#include <core/util.h>

void io_readFile(vm_t* vm) {
	char* path = AS_STRING(vm_pop(vm));
	char* buffer = readFile(path);
    vm_register(vm, (!buffer) ? STRING_VAL("") : STRING_NOCOPY_VAL(buffer));
}

void io_writeFile(vm_t* vm) {
	char* mode = AS_STRING(vm_pop(vm));
	char* content = AS_STRING(vm_pop(vm));
	char* filename = AS_STRING(vm_pop(vm));

	FILE* fp = fopen(filename, mode);
	if(fp) {
        fprintf(fp, "%s", content);
    	fclose(fp);
    }
	vm_push(vm, NULL_VAL);
}

int io_gen_signatures(list_t* toplevel) {
	signature_new();
	require_func();

	// readFile(str:char[]) -> char[]
	function_new("readFile", DATA_STRING, 20);
	function_add_param(0, DATA_STRING);
	function_upload(toplevel);

	// writeFile(name:char[], content:char[], mode:char[]) -> void
	function_new("writeFile", DATA_VOID, 21);
	function_add_param(0, DATA_STRING);
	function_add_param(0, DATA_STRING);
	function_add_param(0, DATA_STRING);
	function_upload(toplevel);

	/**
	File class API

	type File(name:char[]) {
		@Getter
		let filename = name

		func read() -> char[] {
			return readFile(filename)
		}

		func write(str:char[]) -> void {
			writeFile(filename, str, "wb")
		}
	}
	**/

	/**
	add:
	func append(str:char[]) -> void {
		writeFile(filename, str, ["a"])
	}
	**/

	ast_t *var = 0, *clazz = 0, *ann = 0;

	// Class File(name:char[]) {
	class_new("File");
	class_add_param("name", DATA_STRING);

	// @Getter
	annotation_new(ANN_GETTER);
	annotation_upload(clazz->classstmt.body);

	// let filename = name
	variable_new("filename", DATA_STRING);
	var->vardecl.initializer = ast_class_create(AST_IDENT, loc);
	var->vardecl.initializer->ident = "name";
	variable_upload(clazz->classstmt.body);

	// read() -> char[] {
	func = ast_class_create(AST_DECLFUNC, loc);
    func->funcdecl.name = "read";
    func->funcdecl.impl.formals = list_new();
    func->funcdecl.impl.body = list_new();
    func->funcdecl.rettype = datatype_new(DATA_STRING);
    func->funcdecl.external = 0;

	// return<call<readFile, filename>>
	ast_t* ret = ast_class_create(AST_RETURN, loc);
	ast_t* callFunc = ast_class_create(AST_CALL, loc);
	callFunc->call.callee = ast_class_create(AST_IDENT, loc);
	callFunc->call.callee->ident = "readFile";
	callFunc->call.args = list_new();

	// readFile (filename)
	param = ast_class_create(AST_IDENT, loc);
	param->ident = "filename";
	list_push(callFunc->call.args, param);

	// return -> readFile(filename)
	// and upload
	ret->returnstmt = callFunc;
	list_push(func->funcdecl.impl.body, ret);
	list_push(clazz->classstmt.body, func);


	// write(str:char[]) -> void
	func = ast_class_create(AST_DECLFUNC, loc);
	func->funcdecl.name = "write";
    func->funcdecl.impl.formals = list_new();
    func->funcdecl.impl.body = list_new();
    func->funcdecl.rettype = datatype_new(DATA_VOID);
    func->funcdecl.external = 0;

	function_add_param("str", DATA_STRING);
	callFunc = ast_class_create(AST_CALL, loc);
	callFunc->call.callee = ast_class_create(AST_IDENT, loc);
	callFunc->call.callee->ident = "writeFile";
	callFunc->call.args = list_new();

	ast_t* p1 = ast_class_create(AST_IDENT, loc);
	p1->ident = "filename";
	ast_t* p2 = ast_class_create(AST_IDENT, loc);
	p2->ident = "str";
	ast_t* p3 = ast_class_create(AST_STRING, loc);
	p3->string = "wb";

	list_push(callFunc->call.args, p1);
	list_push(callFunc->call.args, p2);
	list_push(callFunc->call.args, p3);
	list_push(func->funcdecl.impl.body, callFunc);
	list_push(clazz->classstmt.body, func);
	class_upload(toplevel);

	return 0;
}
