#ifndef libdef_h
#define libdef_h

// Macros for library creation
#include <parser/ast.h>

#define SIGNATURE_BEGIN() \
    location_t loc = {0, 0};
    ast_t* param = 0;
    ast_t* fn = 0;

#define FUNCTION_NEW(nm, rtype) \
    fn = ast_class_create(AST_DECLFUNC, loc); \
    fn->funcdecl.name = strdup(nm); \
    fn->funcdecl.impl.formals = list_new(); \
    fn->funcdecl.impl.body = 0; \
    fn->funcdecl.rettype = datatype_new(rtype); \
    fn->funcdecl.external = true;

#define FUNCTION_PUSH(node) \
    list_push(toplevel, fn);

#define ADD_PARAM(dtype) \
    param = ast_class_create(AST_DECLVAR, loc); \
	param->vardecl.name = 0; \
	param->vardecl.type = datatype_new(dtype); \
	list_push(fn->funcdecl.impl.formals, param);

#endif
