// Copyright (C) 2017 Alexander Koch
#include "lexer.h"

const char* token_string(token_type_t type) {
    switch(type) {
        case TOKEN_EOF: return "<eof>";
        case TOKEN_NEWLINE: return "newline";
        case TOKEN_WORD: return "word";
        case TOKEN_STRING: return "string";
        case TOKEN_INT: return "int";
        case TOKEN_FLOAT: return "float";
        case TOKEN_BOOL: return "bool";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_LBRACE: return "{";
        case TOKEN_RBRACE: return "}";
        case TOKEN_LBRACKET: return "[";
        case TOKEN_RBRACKET: return "]";
        case TOKEN_COMMA: return ",";
        case TOKEN_SEMICOLON: return ";";
        case TOKEN_ADD: return "+";
        case TOKEN_SUB: return "-";
        case TOKEN_MUL: return "*";
        case TOKEN_DIV: return "/";
        case TOKEN_MOD: return "%";
        case TOKEN_EQUAL: return "=";
        case TOKEN_ASSIGN: return ":=";
        case TOKEN_NEQUAL: return "!=";
        case TOKEN_NOT: return "!";
        case TOKEN_DOT: return ".";
        case TOKEN_BITLSHIFT: return "<<";
        case TOKEN_BITRSHIFT: return ">>";
        case TOKEN_LEQUAL: return "<=";
        case TOKEN_GEQUAL: return ">=";
        case TOKEN_LESS: return "<";
        case TOKEN_GREATER: return ">";
        case TOKEN_AND: return "&&";
        case TOKEN_OR: return "||";
        case TOKEN_BITAND: return "&>";
        case TOKEN_BITOR: return "|";
        case TOKEN_BITXOR: return "^";
        case TOKEN_BITNOT: return "~";
        case TOKEN_DOUBLECOLON: return "::";
        case TOKEN_COLON: return ":";
        case TOKEN_ARROW: return "->";
        case TOKEN_AT: return "@";
        case TOKEN_USING: return "using";
        case TOKEN_LET: return "let";
        case TOKEN_MUT: return "mut";
        case TOKEN_FUNC: return "func";
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_WHILE: return "while";
        case TOKEN_TYPE: return "type";
        case TOKEN_RETURN: return "return";
        case TOKEN_NONE: return "None";
        default: return "undefined";
    }
}

void lex_error(lexer_t* lexer, const char* err) {
    lexer->error = 1;
    printf("%s:%d:%d (Lexis): %s\n", lexer->name, lexer->location.line, lexer->location.column, err);
}

#define RESERVED_ENTRY(w, t) {w, sizeof(w) - 1, t}

typedef struct {
    const char* str;
    size_t len;
    token_type_t type;
} Reserved;

token_type_t keywords[] = {
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
};

int is_special(char c) {
    switch(c) {
        case '@':
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

void lex_skip_space(lexer_t* lexer) {
    if(*lexer->cursor == '#') {
        while(*lexer->cursor != '\n' && *lexer->cursor != '\r') {
            lexer->cursor++;
        }
    }

    while(isspace(*lexer->cursor)
        && *lexer->cursor != '\n'
        && *lexer->cursor != '\r'
        && *lexer->cursor != '#') {
        lexer->cursor++;
    }
}

int is_word_start(char c) {
    return isalpha(c) || c == '_';
}

int is_word(char c) {
    return isalnum(c) || c == '_';
}

int is_space(lexer_t* lexer) {
    return isspace(lexer->cursor[0]) || lexer->cursor[0] == '#';
}

int is_punct(lexer_t* lexer) {
    return is_special(lexer->cursor[0]);
}

int is_number(lexer_t* lexer) {
    return isdigit(lexer->cursor[0]);
}

int is_word_begin(lexer_t* lexer) {
    return is_word_start(lexer->cursor[0]);
}

int is_string_begin(lexer_t* lexer) {
    return lexer->cursor[0] == '"';
}

int is_eof(lexer_t* lexer) {
    return lexer->cursor[0] == 0;
}

int lex_space(lexer_t* lexer, token_t* token) {
    lex_skip_space(lexer);

    token->value = 0;
    if(*lexer->cursor == '\r') {
        lexer->cursor++;
    }

    if(*lexer->cursor == '\n') {
        lexer->location.line++;
        lexer->lastline = lexer->cursor++;

        switch(lexer->last_type) {
            case TOKEN_WORD:
            case TOKEN_STRING:
            case TOKEN_INT:
            case TOKEN_FLOAT:
            case TOKEN_BOOL:
            case TOKEN_RETURN:
            case TOKEN_RPAREN:
            case TOKEN_RBRACKET:
            case TOKEN_RBRACE: {
                token->type = TOKEN_SEMICOLON;
                return 1;
            }
            default: break;
        }
    }

    token->type = TOKEN_NEWLINE;
    return 1;
}

int lex_op(lexer_t* lexer, token_t* token) {
    static const Reserved ops[] = {
        RESERVED_ENTRY("(", TOKEN_LPAREN),
        RESERVED_ENTRY(")", TOKEN_RPAREN),
        RESERVED_ENTRY("[", TOKEN_LBRACKET),
        RESERVED_ENTRY("]", TOKEN_RBRACKET),
        RESERVED_ENTRY("{", TOKEN_LBRACE),
        RESERVED_ENTRY("}", TOKEN_RBRACE),
        RESERVED_ENTRY(",", TOKEN_COMMA),
        RESERVED_ENTRY(";", TOKEN_SEMICOLON),
        RESERVED_ENTRY("+", TOKEN_ADD),
        RESERVED_ENTRY("->", TOKEN_ARROW),
        RESERVED_ENTRY("-", TOKEN_SUB),
        RESERVED_ENTRY("*", TOKEN_MUL),
        RESERVED_ENTRY("/", TOKEN_DIV),
        RESERVED_ENTRY("%", TOKEN_MOD),
        RESERVED_ENTRY("=", TOKEN_EQUAL),
        RESERVED_ENTRY(":=", TOKEN_ASSIGN),
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
        RESERVED_ENTRY("~", TOKEN_BITNOT),
        RESERVED_ENTRY("::", TOKEN_DOUBLECOLON),
        RESERVED_ENTRY(":", TOKEN_COLON),
        RESERVED_ENTRY("@", TOKEN_AT)
    };

    size_t i;
    for(i = 0; i < sizeof(ops)/sizeof(ops[0]); i++) {
        if(strncmp(lexer->cursor, ops[i].str, ops[i].len) == 0) {
            token->type = ops[i].type;
            token->value = strndup(lexer->cursor, ops[i].len);
            lexer->cursor += ops[i].len;
            return 1;
        }
    }
    return 0;
}

int lex_num(lexer_t* lexer, token_t* token) {
    int is_float = 0;
    const char* end = lexer->cursor;
    while(isdigit(end[0])) end++;

    if(end[0] == '.' && isdigit(end[1])) {
        is_float = 1;
        end++;

        while(isdigit(end[0])) end++;
    }

    token->type = is_float ? TOKEN_FLOAT : TOKEN_INT;
    token->value = strndup(lexer->cursor, end - lexer->cursor);
    lexer->cursor = end;
    return 1;
}

int lex_word(lexer_t* lexer, token_t* token) {
    const char* end = lexer->cursor;
    while(is_word(end[0])) end++;

    token->type = TOKEN_WORD;
    token->value = strndup(lexer->cursor, end - lexer->cursor);
    lexer->cursor = end;

    if(!strcmp(token->value, "true") || !strcmp(token->value, "false")) {
        token->type = TOKEN_BOOL;
    }

    // Parse the keywords
    for(unsigned i = 0; i < sizeof(keywords)/sizeof(token_type_t); i++) {
        if(!strcmp(token->value, token_string(keywords[i]))) {
            token->type = keywords[i];
        }
    }

    return 1;
}

int lex_string(lexer_t* lexer, token_t* token) {
    lexer->cursor++;
    bytebuffer_t buffer;
    bytebuffer_init(&buffer);

    while(lexer->cursor[0] != '"') {
        switch(lexer->cursor[0]) {
            case 0:
                lex_error(lexer, "Never ending string");
                return 0;
            case '\n':
            case '\r': {
                lex_error(lexer, "Strings may not contain newlines");
                return 0;
            }
            case '\\': {
                lexer->cursor++;
                switch(lexer->cursor[0]) {
                    case '"':
                        bytebuffer_write(&buffer, '"');
                        break;
                    case '\\':
                        bytebuffer_write(&buffer, '\\');
                        break;
                    case 'b':
                        bytebuffer_write(&buffer, '\b');
                        break;
                    case 'n':
                        bytebuffer_write(&buffer, '\n');
                        break;
                    case 'r':
                        bytebuffer_write(&buffer, '\r');
                        break;
                    case 't':
                        bytebuffer_write(&buffer, '\t');
                        break;
                    case 'v':
                        bytebuffer_write(&buffer, '\v');
                        break;
                    case 'f':
                        bytebuffer_write(&buffer, '\f');
                        break;
                    case '0':
                    default: {
                        lex_error(lexer, "Invalid escape character");
                        bytebuffer_clear(&buffer);
                        return 0;
                    }
                }
                lexer->cursor++;
                break;
            }
            default: {
                bytebuffer_write(&buffer, lexer->cursor[0]);
                lexer->cursor++;
                break;
            }
        }
    }

    lexer->cursor++; // end of quotation mark
    token->type = TOKEN_STRING;
    token->value = strndup((char*)buffer.data, buffer.count);
    bytebuffer_clear(&buffer);
    return 1;
}

int next_token(lexer_t* lexer, token_t* token) {
    lexer->location.column = lexer->cursor - lexer->lastline + 1;
    token->location.line = lexer->location.line;
    token->location.column = lexer->location.column;

    if(is_space(lexer)) {
        return lex_space(lexer, token);
    }
    if(is_punct(lexer)) {
        return lex_op(lexer, token);
    }
    if(is_number(lexer)) {
        return lex_num(lexer, token);
    }
    if(is_word_begin(lexer)) {
        return lex_word(lexer, token);
    }
    if(is_string_begin(lexer)) {
        return lex_string(lexer, token);
    }
    if(is_eof(lexer)) {
        return 0;
    }

    lex_error(lexer, "Invalid token / symbol");
    return 0;
}

token_t* lexer_scan(const char* name, const char* src, size_t* numTokens) {
    lexer_t lexer;
    lexer.location.line = 1;
    lexer.location.column = 1;
    lexer.name = name;
    lexer.source = src;
    lexer.cursor = src;
    lexer.lastline = src;
    lexer.error = 0;
    lexer.last_type = TOKEN_EOF;

    size_t alloc_size = 8;
    size_t n = 0;
    token_t* buffer = malloc(alloc_size * sizeof(token_t));
    if(!buffer) return 0;

    token_t token;
    while(next_token(&lexer, &token)) {
        if(token.type == TOKEN_NEWLINE) {
            continue;
        }

        if(n >= alloc_size) {
            // resize buffer
            alloc_size *= 2;
            buffer = realloc(buffer, alloc_size * sizeof(buffer[0]));
        }

        buffer[n++] = token;
        lexer.last_type = token.type;
    }

    if(lexer.error) {
        lexer_free_buffer(buffer, n);
        return 0;
    }

    *numTokens = n;
    return buffer;
}

void lexer_print_tokens(token_t* tokens, size_t n) {
    for(size_t i = 0; i < n; i++) {
        token_t* token = &tokens[i];
        if(token->type == TOKEN_SEMICOLON) {
            printf("[TOK: Semicolon]\n");
        } else {
            printf("[TOK: %s], ", token->value);
        }
    }
    printf("\n");
}

void lexer_free_buffer(token_t* buffer, size_t n) {
    for(size_t i = 0; i < n; i++) {
        free(buffer[i].value);
    }
    free(buffer);
}
