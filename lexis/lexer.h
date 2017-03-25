/**
 * lexer.h
 * @author Alexander Koch
 * @desc Lexical analysis tool.
 * Reads a string and converts its content into fragments: Tokens.
 * Each token has a certain type with a string value.
 * The string value may be NULL.
 */

#ifndef lexer_h
#define lexer_h

#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <core/mem.h>
#include <core/util.h>
#include <adt/vector.h>
#include <adt/bytebuffer.h>

// Location in file / code location
typedef struct {
    unsigned int line;
    unsigned int column;
} location_t;

// Lexer structure
typedef struct lexer_t {
    location_t location;
    const char* name;
    const char* source;
    const char* cursor;
    const char* lastline;
    int error;
} lexer_t;

typedef enum {
    TOKEN_EOF,
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
    TOKEN_ADD,       // '+' OP
    TOKEN_SUB,       // '-' OP
    TOKEN_MUL,       // '*' OP
    TOKEN_DIV,       // '/' OP
    TOKEN_MOD,       // '%' OP
    TOKEN_EQUAL,     // '=' OP
    TOKEN_ASSIGN,    // ':=' OP
    TOKEN_NEQUAL,    // '!=' OP
    TOKEN_NOT,       // '!' OP
    TOKEN_DOT,       // '.'
    TOKEN_BITLSHIFT, // '<<' OP
    TOKEN_BITRSHIFT, // '>>' OP
    TOKEN_LEQUAL,    // '<=' OP
    TOKEN_GEQUAL,    // '>=' OP
    TOKEN_LESS,      // '<' OP
    TOKEN_GREATER,   // '>' OP
    TOKEN_AND,       // '&&' OP
    TOKEN_OR,        // '||' OP
    TOKEN_BITAND,    // '&' OP
    TOKEN_BITOR,     // '|' OP
    TOKEN_BITXOR,    // '^' OP
    TOKEN_BITNOT,    // '~' OP
    TOKEN_DOUBLECOLON, // '::'
    TOKEN_COLON,     // ':'
    TOKEN_ARROW,     // '->'
    TOKEN_AT,        // '@'

    // Keywords
    TOKEN_USING,
    TOKEN_LET,
    TOKEN_MUT,
    TOKEN_FUNC,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_TYPE,
    TOKEN_RETURN,
    TOKEN_NONE,
} token_type_t;

typedef struct {
    location_t location;
    token_type_t type;
    char* value;
} token_t;

static const token_t EOF_TOKEN = {
    .location = {.line = 0, .column = 0},
    .type = TOKEN_EOF,
    .value = "<EOF>"
};

const char* token_string(token_type_t type);

token_t* lexer_scan(const char* name, const char* src, size_t* numTokens);
void lexer_print_tokens(token_t* tokens, size_t n);
void lexer_free_buffer(token_t* buffer, size_t n);

#endif
