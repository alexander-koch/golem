/**
 *  parser.h
 *  @author Alexander Koch
 *  @desc Parser structure for analysis of tokens.
 */

#ifndef parser_h
#define parser_h

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#include <core/mem.h>
#include <lexis/lexer.h>
#include <parser/ast.h>
#include <adt/vector.h>

// Parser structure
// ----------
// @name Name of the current parser (filename)
// @buffer Buffer that keeps the tokens
// @num_tokens Length of the token buffer
// @cursor Current position within the buffer
// @error Error flag
// @top Highest AST node in the tree (toplevel)

// Usage
// ----------
// const char* content = "your/code/as/text/here";
//
// parser_t parser;
// parser_init(&parser);
// ast_t* root = parser_run(&parser, content);
//
// <Do something with root>
//
// parser_free(&parser);

typedef struct parser_t
{
    const char* name;
    token_t* buffer;
    size_t num_tokens;
    size_t cursor;
    bool error;
    ast_t* top;
} parser_t;

// Parser functions
void parser_init(parser_t* parser, const char* name);
ast_t* parser_run(parser_t* parser, const char* content);
void parser_free(parser_t* parser);

// Parser helper functions
ast_t* parse_stmt(parser_t* parser);
void parser_throw(parser_t* parser, const char* format, ...);
int parser_error(parser_t* parser);

#endif
