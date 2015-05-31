#include "lexer.h"

void lexer_init(lexer_t* lexer)
{
    lexer->location.line = 0;
    lexer->location.column = 0;
    lexer->source = 0;
    lexer->cursor = 0;
    lexer->lastline = 0;
    lexer->error = 0;
    lexer->eof = 0;
}

void lex_error(lexer_t* lexer, const char* err)
{
    lexer->error = 1;
    fprintf(stdout, "[line %d, column %d] Lexical error: %s\n", lexer->location.line, lexer->location.column, err);
}

#define RESERVED_ENTRY(w, t) {w, sizeof(w) - 1, t}

typedef struct
{
    const char* str;
    size_t len;
    token_type_t type;
} Reserved;

int is_special(char c)
{
    switch(c)
    {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '=':
        case '!':
        case '?':
        case ':':
        case '.':
        case ',':
        case ';':
        case '<':
        case '>':
        case '&':
        case '|':
        case '^':
        case '~':
        case '$':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}': return 1;
        default: return 0;
    }
}

void lex_skip_space(lexer_t* lexer)
{
    if(lexer->cursor[0] == '#')
    {
        while(lexer->cursor[0] != '\n' && lexer->cursor[0] != '\r')
        {
            lexer->cursor++;
        }
    }

    while(isspace(lexer->cursor[0]) && lexer->cursor[0] != '\n' && lexer->cursor[0] != '\r' && lexer->cursor[0] != '#')
    {
        lexer->cursor++;
    }
}

int is_word_start(char c)
{
    return isalpha(c) || c == '_';
}

int is_word(char c)
{
    return isalnum(c) || c == '_';
}

int is_newline(lexer_t* lexer)
{
    return lexer->cursor[0] == '\r' || lexer->cursor[0] == '\n';
}

int is_space(lexer_t* lexer)
{
    return isspace(lexer->cursor[0]) || lexer->cursor[0] == '#';
}

int is_punct(lexer_t* lexer)
{
    return is_special(lexer->cursor[0]);
}

int is_number(lexer_t* lexer)
{
    return isdigit(lexer->cursor[0]);
}

int is_word_begin(lexer_t* lexer)
{
    return is_word_start(lexer->cursor[0]);
}

int is_string_begin(lexer_t* lexer)
{
    return lexer->cursor[0] == '"';
}

int is_eof(lexer_t* lexer)
{
    return lexer->cursor[0] == 0;
}

int lex_newline(lexer_t* lexer, token_t* token)
{
    if(lexer->cursor[0] == '\n')
    {
        if(lexer->cursor[1] == '\r')
        {
            lexer->cursor++;
        }
        lexer->location.line++;
        lexer->lastline = lexer->cursor++;
    }
    else if(lexer->cursor[0] == '\r')
    {
        if(lexer->cursor[1] == '\n')
        {
            lexer->cursor++;
        }
        lexer->location.line++;
        lexer->lastline = lexer->cursor++;
    }

    token->type = TOKEN_NEWLINE;
    token->value = 0;
    return 1;
}

int lex_space(lexer_t* lexer, token_t* token)
{
    lex_skip_space(lexer);
    token->type = TOKEN_SPACE;
    token->value = 0;
    return 1;
}

int lex_op(lexer_t* lexer, token_t* token)
{
    static const Reserved ops[] =
    {
        RESERVED_ENTRY("(", TOKEN_LPAREN),
        RESERVED_ENTRY(")", TOKEN_RPAREN),
        RESERVED_ENTRY("[", TOKEN_LBRACKET),
        RESERVED_ENTRY("]", TOKEN_RBRACKET),
        RESERVED_ENTRY("{", TOKEN_LBRACE),
        RESERVED_ENTRY("}", TOKEN_RBRACE),
        RESERVED_ENTRY(",", TOKEN_COMMA),
        RESERVED_ENTRY(";", TOKEN_SEMICOLON),
        RESERVED_ENTRY("+", TOKEN_ADD),
        RESERVED_ENTRY("-", TOKEN_SUB),
        RESERVED_ENTRY("*", TOKEN_MUL),
        RESERVED_ENTRY("/", TOKEN_DIV),
        RESERVED_ENTRY("%", TOKEN_MOD),
        RESERVED_ENTRY("==", TOKEN_EQUAL),
        RESERVED_ENTRY("=", TOKEN_ASSIGN),
        RESERVED_ENTRY("!=", TOKEN_NEQUAL),
        RESERVED_ENTRY("!", TOKEN_NOT),
        RESERVED_ENTRY(".", TOKEN_DOT),
        RESERVED_ENTRY("<<", TOKEN_BITLSHIFT),
        RESERVED_ENTRY("<=", TOKEN_LEQUAL),
        RESERVED_ENTRY("<", TOKEN_LESS),
        RESERVED_ENTRY(">>", TOKEN_BITRSHIFT),
        RESERVED_ENTRY(">=", TOKEN_GEQUAL),
        RESERVED_ENTRY(">", TOKEN_GREATER),
        RESERVED_ENTRY("&&", TOKEN_AND),
        RESERVED_ENTRY("&", TOKEN_BITAND),
        RESERVED_ENTRY("||", TOKEN_OR),
        RESERVED_ENTRY("|", TOKEN_BITOR),
        RESERVED_ENTRY("^", TOKEN_BITXOR),
        RESERVED_ENTRY("~", TOKEN_BITNOT)
    };

    size_t i;
    for(i = 0; i < sizeof(ops)/sizeof(ops[0]); i++)
    {
        if(strncmp(lexer->cursor, ops[i].str, ops[i].len) == 0)
        {
            token->type = ops[i].type;
            token->value = strndup(lexer->cursor, ops[i].len);
            lexer->cursor += ops[i].len;
            return 1;
        }
    }
    return 0;
}

int lex_num(lexer_t* lexer, token_t* token)
{
    int is_float = 0;
    const char* end = lexer->cursor;
    while(isdigit(end[0])) end++;

    if(end[0] == '.')
    {
        is_float = 1;
        end++;

        while(isdigit(end[0])) end++;
    }

    token->type = is_float ? TOKEN_FLOAT : TOKEN_INT;
    token->value = strndup(lexer->cursor, end - lexer->cursor);
    lexer->cursor = end;
    return 1;
}

int lex_word(lexer_t* lexer, token_t* token)
{
    const char* end = lexer->cursor;
    while(is_word(end[0])) end++;

    token->type = TOKEN_WORD;
    token->value = strndup(lexer->cursor, end - lexer->cursor);
    lexer->cursor = end;

    if(!strcmp(token->value, "true") || !strcmp(token->value, "false"))
    {
        token->type = TOKEN_BOOL;
    }

    return 1;
}

int lex_string(lexer_t* lexer, token_t* token)
{
    const char* begin = lexer->cursor++ + 1;

    while(lexer->cursor[0] != '"')
    {
        if(lexer->cursor[0] == 0)
        {
            lex_error(lexer, "Never ending string");
            return 0;
        }
        if(lexer->cursor[0] == '\n' || lexer->cursor[0] == '\r')
        {
            lex_error(lexer, "Strings may not contain newlines");
            return 0;
        }
        else
        {
            lexer->cursor++;
        }
    }

    lexer->cursor++; // end of quotation mark
    token->type = TOKEN_STRING;
    token->value = strndup(begin, lexer->cursor - begin - 1);
    return 1;
}

int next_token(lexer_t* lexer, token_t* token)
{
    lexer->location.column = lexer->cursor - lexer->lastline + 1;
    token->location.line = lexer->location.line;
    token->location.column = lexer->location.column;

    if(is_newline(lexer))
    {
        return lex_newline(lexer, token);
    }
    if(is_space(lexer))
    {
        return lex_space(lexer, token);
    }
    if(is_punct(lexer))
    {
        return lex_op(lexer, token);
    }
    if(is_number(lexer))
    {
        return lex_num(lexer, token);
    }
    if(is_word_begin(lexer))
    {
        return lex_word(lexer, token);
    }
    if(is_string_begin(lexer))
    {
        return lex_string(lexer, token);
    }
    if(is_eof(lexer))
    {
        lexer->eof = 1;
        return 0;
    }

    lex_error(lexer, "Fatal error, could not interpret token");
    return 0;
}

token_t* lexer_lex(lexer_t* lexer, const char* src, size_t* numTokens)
{
    lexer->location.line = 1;
    lexer->location.column = 1;
    lexer->source = src;
    lexer->lastline = src;
    lexer->eof = 0;
    lexer->error = 0;
    lexer->cursor = src;

    size_t alloc_size = 8;
    size_t n = 0;
    token_t* buffer = malloc(alloc_size * sizeof(token_t));
    assert(buffer);

    token_t token;
    while(next_token(lexer, &token))
    {
        if(token.type == TOKEN_SPACE)
        {
            continue;
        }

        if(n >= alloc_size)
        {
            // resize buffer
            alloc_size *= 2;
            buffer = realloc(buffer, alloc_size * sizeof(buffer[0]));
        }

        buffer[n++] = token;
    }

    if(lexer->error)
    {
        lexer_free_buffer(buffer, n);
        return 0;
    }

    *numTokens = n;
    return buffer;
}

void lexer_print_tokens(token_t* tokens, size_t n)
{
    size_t i;
    for(i = 0; i < n; i++)
    {
        token_t* token = &tokens[i];
        if(token->type == TOKEN_NEWLINE)
        {
            fprintf(stdout, "[TOK: NEWLINE]\n");
        }
        else
        {
            fprintf(stdout, "[TOK: %s], ", token->value);
        }
    }
    fprintf(stdout, "\n");
}

void lexer_free_buffer(token_t* buffer, size_t n)
{
    size_t i;
    for(i = 0; i < n; i++)
    {
        free(buffer[i].value);
    }
    free(buffer);
}
