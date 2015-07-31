#ifndef ast_h
#define ast_h

#include <stddef.h>
#include <stdbool.h>

#include <lexis/lexer.h>
#include <adt/list.h>
#include <core/mem.h>
#include <core/api.h>

// needed types? i32, f32, i64, f64, bool, str, custom classes
// types: void, bool, str, int, float, custom classes

typedef enum
{
    DATA_NULL = 1,// 0x1,
    DATA_BOOL = 2, //0x2,
    DATA_INT = 3, //0x4,
    DATA_FLOAT = 4, //0x8,
    DATA_STRING = 5, //0x10,
    DATA_OBJECT = 6, //0x20,
    DATA_VOID = 7, //0x40
    DATA_LAMBDA = 8 // 0x80
} datatype_t;

typedef struct
{
    char* name;
    datatype_t type;
    location_t loc;
} param_t;

typedef struct ast_s ast_t;

typedef enum
{
    AST_NULL,
    AST_IDENT,
    AST_FLOAT,
    AST_INT,
    AST_BOOL,
    AST_STRING,
    AST_BINARY,
    AST_UNARY,
    AST_SUBSCRIPT,
    AST_CALL,
    AST_DECLVAR,
    AST_DECLFUNC,
    AST_IF,
    AST_IFCLAUSE,
    AST_WHILE,
    AST_INCLUDE,
    AST_CLASS,
    AST_RETURN,
    AST_TOPLEVEL,
} ast_class_t;

typedef struct
{
    ast_t* cond;
    list_t* body;
} ast_cond_t;

typedef struct
{
    ast_t* key;
    ast_t* expr;
} ast_field_t;

typedef struct
{
    list_t* formals;
    list_t* body;
} ast_lambda_t;

typedef struct
{
    char* name;
    ast_lambda_t impl;
    datatype_t rettype;
    bool external;
} ast_func_t;

typedef struct
{
    char* name;
    bool mutate;
    ast_t* initializer;
    datatype_t type;
} ast_decl_t;

typedef struct
{
    char* name;
    list_t* body;
} ast_struct_t;

struct ast_s
{
    ast_class_t class;
    location_t location;
    union
    {
        char* ident;
        char* string;
        list_t* toplevel;
        list_t* ifstmt;
        I64 i;
        F64 f;
        bool b;
        ast_t* include;
        ast_t* returnstmt;
        ast_field_t subscript;
        ast_func_t funcdecl;
        ast_decl_t vardecl;
        ast_cond_t ifclause;
        ast_cond_t whilestmt;
        ast_struct_t classstmt;

        struct
        {
            token_type_t op;
            ast_t* left;
            ast_t* right;
        } binary;

        struct
        {
            token_type_t op;
            ast_t* expr;
        } unary;

        struct
        {
            ast_t* callee;
            list_t* args;
        } call;
    };
};

const char* datatype2str(datatype_t type);

#define ast_is_number(x) (x->class == AST_FLOAT || x->class == AST_INT)

ast_t* ast_class_create(ast_class_t class, location_t location);
const char* ast_classname(ast_class_t class);
void ast_free(ast_t* ast);

#endif
