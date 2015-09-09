#ifndef parser_h
#define parser_h

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#include <core/mem.h>
#include <core/api.h>
#include <lexis/lexer.h>
#include <parser/ast.h>
#include <adt/vector.h>

typedef struct
{
    const char* name;
    lexer_t lexer;
    token_t* buffer;
    size_t num_tokens;
    size_t cursor;
    bool error;
    ast_t* top;
    vector_t* docs;
} parser_t;

void parser_init(parser_t* parser, const char* name);
ast_t* parse_stmt(parser_t* parser);
ast_t* parser_run(parser_t* parser, const char* content);
void parser_free(parser_t* parser);
void parser_throw(parser_t* parser, const char* format, ...);
int parser_error(parser_t* parser);

#endif
