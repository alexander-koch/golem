#ifndef lexer_h
#define lexer_h

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <core/mem.h>
#include <core/util.h>

typedef struct
{
    unsigned line;
    unsigned column;
} location_t;

typedef struct
{
    location_t location;
    const char* source;
    const char* cursor;
    const char* lastline;
    int error;
    int eof;
} lexer_t;

typedef enum
{
    TOKEN_NEWLINE,
    TOKEN_SPACE,
    TOKEN_WORD,
    TOKEN_STRING,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_BOOL,
    TOKEN_LPAREN,    // '('
    TOKEN_RPAREN,    // ')'
    TOKEN_LBRACE,    // '{'
    TOKEN_RBRACE,    // '}'
    TOKEN_LBRACKET,  // '['
    TOKEN_RBRACKET,  // ']'
    TOKEN_COMMA,     // ','
    TOKEN_SEMICOLON, // ';'
    TOKEN_ADD,       // '+'
    TOKEN_SUB,       // '-'
    TOKEN_MUL,       // '*'
    TOKEN_DIV,
    TOKEN_MOD,
    TOKEN_EQUAL,
    TOKEN_ASSIGN,
    TOKEN_NEQUAL,
    TOKEN_NOT,
    TOKEN_DOT,
    TOKEN_BITLSHIFT,
    TOKEN_BITRSHIFT,
    TOKEN_LEQUAL,
    TOKEN_GEQUAL,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_BITAND,
    TOKEN_BITOR,
    TOKEN_BITXOR,
    TOKEN_BITNOT
} token_type_t;

typedef struct
{
    location_t location;
    token_type_t type;
    char* value;
} token_t;

void lexer_init(lexer_t* lexer);
token_t* lexer_lex(lexer_t* lexer, const char* src, size_t* numTokens);
void lexer_print_tokens(token_t* tokens, size_t n);
void lexer_free_buffer(token_t* buffer, size_t n);

#endif
