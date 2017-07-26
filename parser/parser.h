/**
 * parser.h
 * Copyright (C) 2017 Alexander Koch
 * Parser structure for analysis of tokens.
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
#include <parser/types.h>
#include <adt/vector.h>

/**
 * # Parser structure
 * @name Name of the current parser (filename)
 * @buffer Buffer that keeps the tokens
 * @num_tokens Length of the token buffer
 * @cursor Current position within the buffer
 * @error Error flag
 * @context Type context
 *
 * ## Usage
 * ```
 * parser_t* parser = parser_new(name, context);
 * ast_t* root = parser_run(&parser, content);
 * parser_free(parser);
 *
 * <Do something with root>
 *
 * ast_free(root);
 * ```
 */

typedef struct parser_t {
    const char* name;
    token_t* buffer;
    size_t num_tokens;
    size_t cursor;
    bool error;
    context_t* context;
} parser_t;

// Parser functions
parser_t* parser_new(const char* name, context_t* context);
ast_t* parser_run(parser_t* parser, char* content);
void parser_free(parser_t* parser);
int parser_error(parser_t* parser);

#endif
