#ifndef libdef_h
#define libdef_h

// Macros for library creation
#ifdef DLL_EXPORT

#define USE_MEM_IMPLEMENTATION
#define GOLEM_API __declspec(dllexport)
#include <core/api.h>
#else
#define GOLEM_API
#endif

// Main stuff
#include <parser/ast.h>
#include <vm/val.h>

#define signature_new() \
    location_t loc = {0, 0}

#define require_class() \
    ast_t* clazz = 0

#define require_func() \
    ast_t* param = 0; \
    ast_t* func = 0

#define class_new(nm) \
    clazz = ast_class_create(AST_CLASS, loc); \
    clazz->classstmt.name = nm; \
    clazz->classstmt.body = list_new(); \
    clazz->classstmt.formals = list_new(); \
    clazz->classstmt.fields = hashmap_new()

#define class_add_param(dtype) \
    param = ast_class_create(AST_DECLVAR, loc); \
    param->vardecl.name = 0; \
    param->vardecl.type = datatype_new(dtype); \
    list_push(clazz->classstmt.formals, param)

#define class_add_function() \
    list_push(clazz->classstmt.body, func);

#define class_upload(ref) \
    list_push(ref, clazz)

#define function_new(nm, rtype, idx) \
    func = ast_class_create(AST_DECLFUNC, loc); \
    func->funcdecl.name = strdup(nm); \
    func->funcdecl.impl.formals = list_new(); \
    func->funcdecl.impl.body = 0; \
    func->funcdecl.rettype = datatype_new(rtype); \
    func->funcdecl.external = idx

#define function_add_param(dtype) \
    param = ast_class_create(AST_DECLVAR, loc); \
    param->vardecl.name = 0; \
    param->vardecl.type = datatype_new(dtype); \
    list_push(func->funcdecl.impl.formals, param)

#define function_upload(ref) \
    list_push(ref, func)

#endif
