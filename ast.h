#ifndef ast_h
#define ast_h

#include <stddef.h>
#include <stdbool.h>

#include <lexic/lexer.h>
#include <adt/list.h>
#include <core/mem.h>
#include <core/api.h>

typedef struct ast_s ast_t;

typedef enum
{
    AST_NULL,
    AST_IDENT,
    AST_FLOAT,
    AST_INT,
    AST_STRING,
    AST_ARRAY,
    AST_TABLE,
    AST_TABLEENTRY,
    AST_BINARY,
    AST_UNARY,
    AST_SUBSCRIPT,
    AST_LAMBDA,
    AST_CALL,
    AST_IF,
    AST_IFCLAUSE,
    AST_WHILE,
    AST_DECLVAR,
    AST_DECLFUN,
    AST_TOPLEVEL,
    AST_FOR
} ast_class_t;

typedef struct
{
    char* name;
    bool mutate;
    ast_t* initializer;
} ast_decl_t;

struct ast_s
{
    ast_class_t class;
    location_t location;
    union
    {
        char* ident;
        char* string;
        list_t toplevel;
        long i;
        double f;
        ast_decl_t vardecl;

        struct
        {
            token_type_t op;
            ast_t* left;
            ast_t* right;
        } binary;

        struct
        {
            ast_t* callee;
            list_t args;
        } call;
    };
};

ast_t* ast_class_create(ast_class_t class, location_t location);
const char* ast_classname(ast_class_t class);
void ast_free(ast_t* ast);

#endif
