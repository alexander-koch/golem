#ifndef libdef_h
#define libdef_h

/**
Helper functions for standard libraries
(Meta-programming)
**/

// Macros DLL export
#ifdef DLL_EXPORT

#define USE_MEM_IMPLEMENTATION
#define GOLEM_API __declspec(dllexport)
#include <core/api.h>

#else
#define GOLEM_API
#endif

// Main stuff
#include <parser/ast.h>
#include <parser/types.h>
#include <vm/val.h>

#define signature_new() location_t loc = {0, 0}

#define require_var() ast_t* var = 0

#define require_class() ast_t* clazz = 0

#define require_func() \
    ast_t* param = 0; \
    ast_t* func = 0

#define require_annotation() ast_t* ann = 0

/**
Class related functions

class_new()
class_add_param()
class_add_function()
class_upload()

(FIX: currently two classes at a time is not possible)
**/

#define class_new(nm) \
    clazz = ast_class_create(AST_CLASS, loc); \
    clazz->classstmt.name = nm; \
    clazz->classstmt.body = list_new(); \
    clazz->classstmt.formals = list_new(); \
    clazz->classstmt.fields = hashmap_new()

#define class_add_param(nm, dtype) \
    param = ast_class_create(AST_DECLVAR, loc); \
    param->vardecl.name = nm; \
    param->vardecl.type = dtype; \
    list_push(clazz->classstmt.formals, param)

#define class_add_function() \
    list_push(clazz->classstmt.body, func);

#define class_upload(ref) list_push(ref, clazz)

/**
Function related functions

function_new()
function_add_param();
function_upload();

(Same for functions, two functions at a time is not possible)
**/

#define function_new(nm, rtype, idx) \
    func = ast_class_create(AST_DECLFUNC, loc); \
    func->funcdecl.name = strdup(nm); \
    func->funcdecl.impl.formals = list_new(); \
    func->funcdecl.impl.body = 0; \
    func->funcdecl.rettype = rtype; \
    func->funcdecl.external = idx

#define function_add_param(nm, dtype) \
    param = ast_class_create(AST_DECLVAR, loc); \
    param->vardecl.name = nm; \
    param->vardecl.type = dtype; \
    list_push(func->funcdecl.impl.formals, param)

#define function_upload(ref) list_push(ref, func)

/**
Annotation
**/

#define annotation_new(idx) \
    ann = ast_class_create(AST_ANNOTATION, loc); \
    ann->annotation = idx

#define annotation_upload(ref) list_push(ref, ann);

/**
Variables
**/

#define variable_new(nm, dtype) \
    var = ast_class_create(AST_DECLVAR, loc); \
    var->vardecl.name = nm; \
    var->vardecl.type = dtype

#define variable_upload(ref) list_push(ref, var)

#endif
