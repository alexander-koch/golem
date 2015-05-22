#include "parser.h"

#define KEYWORD_DECLARATION "let"

ast_t* parse_var_declaration(parser_t* parser);

void parser_init(parser_t* parser)
{
    lexer_init(&parser->lexer);
    parser->buffer = 0;
    parser->num_tokens = 0;
    parser->cursor = 0;
    parser->error = 0;
}

// helper functions begin

int parser_end(parser_t* parser)
{
    return parser->cursor >= parser->num_tokens;
}

token_t* current_token(parser_t* parser)
{
    return  &parser->buffer[parser->cursor];
}

location_t get_location(parser_t* parser)
{
    return current_token(parser)->location;
}

int match_string(parser_t* parser, const char* token)
{
    if(parser_end(parser)) return 0;
    return !strcmp(parser->buffer[parser->cursor].value, token);
}

int match_type(parser_t* parser, token_type_t type)
{
    return parser->buffer[parser->cursor].type == type;
}

int match_next(parser_t* parser, const char* token)
{
    if(parser_end(parser) || parser->cursor+1 >= parser->num_tokens) return 0;
    return !strcmp(parser->buffer[parser->cursor+1].value, token);
}

token_t* accept_token(parser_t* parser)
{
    return &parser->buffer[parser->cursor++];
}

token_t* accept_token_string(parser_t* parser, const char* str)
{
    if(match_string(parser, str))
    {
        return &parser->buffer[parser->cursor++];
    }
    return 0;
}

token_t* accept_token_type(parser_t* parser, token_type_t type)
{
    if(parser_end(parser)) return 0;

    if(parser->buffer[parser->cursor].type == type)
    {
        return &parser->buffer[parser->cursor++];
    }
    else
    {
        fprintf(stdout, "Token: %d\n", parser->buffer[parser->cursor].type);
    }
    return 0;
}

token_t* parse_expression_until_type(parser_t* parser, token_type_t type, size_t* tokens)
{
    size_t alloc_size = 8;
    size_t n = 0;
    token_t* buffer = malloc(sizeof(token_t) * alloc_size);
    while(parser->buffer[parser->cursor].type != type && !parser_end(parser))
    {
        if(n >= alloc_size)
        {
            alloc_size *= 2;
            buffer = realloc(buffer, alloc_size * sizeof(buffer[0]));
        }
        buffer[n++] = parser->buffer[parser->cursor];
        parser->cursor++;
    }
    *tokens = n;
    return buffer;
}

token_t* parse_expression_until_string(parser_t* parser, const char* key, size_t* tokens)
{
    size_t alloc_size = 8;
    size_t n = 0;
    token_t* buffer = malloc(sizeof(token_t) * alloc_size);
    while(strcmp(parser->buffer[parser->cursor].value, key) && !parser_end(parser))
    {
        if(n >= alloc_size)
        {
            alloc_size *= 2;
            buffer = realloc(buffer, alloc_size * sizeof(buffer[0]));
        }
        buffer[n++] = parser->buffer[parser->cursor];
        parser->cursor++;
    }
    *tokens = n;
    parser->cursor++;
    return buffer;
}

token_t* parse_expression_until_string_and_type(parser_t* parser, const char* key, token_type_t type, size_t* tokens)
{
    size_t alloc_size = 8;
    size_t n = 0;
    token_t* buffer = malloc(sizeof(token_t) * alloc_size);
    while(strcmp(parser->buffer[parser->cursor].value, key) && parser->buffer[parser->cursor+1].type != type)
    {
        if(n >= alloc_size)
        {
            alloc_size *= 2;
            buffer = realloc(buffer, alloc_size * sizeof(buffer[0]));
        }
        buffer[n++] = parser->buffer[parser->cursor];
        parser->cursor++;
    }
    *tokens = n;
    parser->cursor++;
    return buffer;
}

token_t* parse_expression_until_string_tuple(parser_t* parser, const char* key1, const char* key2, size_t* tokens)
{
    size_t alloc_size = 8;
    size_t n = 0;
    token_t* buffer = malloc(sizeof(token_t) * alloc_size);
    while(strcmp(parser->buffer[parser->cursor].value, key1) && strcmp(parser->buffer[parser->cursor+1].value, key2))
    {
        if(n >= alloc_size)
        {
            alloc_size *= 2;
            buffer = realloc(buffer, alloc_size * sizeof(buffer[0]));
        }
        buffer[n++] = parser->buffer[parser->cursor];
        parser->cursor++;
    }
    *tokens = n;
    parser->cursor++;
    return buffer;
}

void skip_newline(parser_t* parser)
{
    while(parser->buffer[parser->cursor].type == TOKEN_NEWLINE)
    {
        parser->cursor++;
    }
}


void parser_free(parser_t* parser)
{
    lexer_free_buffer(parser->buffer, parser->num_tokens);
}

int parser_error(parser_t* parser)
{
    return parser->error;
}


// real parsing functions begin

int match_simple(parser_t* parser)
{
    return match_type(parser, TOKEN_INT) ||
        match_type(parser, TOKEN_FLOAT) ||
        match_type(parser, TOKEN_STRING);
}

int match_literal(parser_t* parser)
{
    return match_simple(parser) ||
        match_string(parser, "[") ||
        match_string(parser, "{");
}

int parse_precedence(token_t* token)
{
    if(!strcmp(token->value, "=")) return 0;
    else if(!strcmp(token->value, "||")) return 1;
    else if(!strcmp(token->value, "&&")) return 2;
    else if(!strcmp(token->value, "|")) return 3;
    else if(!strcmp(token->value, "^")) return 4;
    else if(!strcmp(token->value, "&")) return 5;
    else if(!strcmp(token->value, "==")) return 6;
    else if(!strcmp(token->value, "!=")) return 6;
    else if(!strcmp(token->value, "<")) return 7;
    else if(!strcmp(token->value, ">")) return 7;
    else if(!strcmp(token->value, "<=")) return 7;
    else if(!strcmp(token->value, ">=")) return 7;
    else if(!strcmp(token->value, "<<")) return 8;
    else if(!strcmp(token->value, ">>")) return 8;
    else if(!strcmp(token->value, "-")) return 9;
    else if(!strcmp(token->value, "+")) return 9;
    else if(!strcmp(token->value, "*")) return 9;
    else if(!strcmp(token->value, "/")) return 9;
    else if(!strcmp(token->value, "%")) return 9;
    return -1;
}

ast_t* parse_simpleliteral(parser_t* parser)
{
    ast_t* node = ast_class_create(-1, get_location(parser));
    if(match_type(parser, TOKEN_FLOAT) || match_type(parser, TOKEN_INT))
    {
        node->class = AST_NUMBER;
        node->number = atof(accept_token(parser)->value);
        return node;
    }
    else if(match_type(parser, TOKEN_STRING))
    {
        node->class = AST_STRING;
        node->string = accept_token(parser)->value;
        return node;
    }
    else
    {
        // TODO: Throw error
    }
    return node;
}

ast_t* parse_literal(parser_t* parser)
{
    if(match_simple(parser))
    {
        return parse_simpleliteral(parser);
    }
    else if(match_string(parser, "["))
    {
        // TODO: Implement
    }
    else if(match_string(parser, "{"))
    {
        // TODO: Implement
    }
    return 0;
}

ast_t* parse_expression_primary(parser_t* parser)
{
    ast_t* ast = 0;
    if(match_literal(parser))
    {
        ast = parse_literal(parser);
    }
    else if(match_type(parser, TOKEN_WORD))
    {
        ast = ast_class_create(AST_IDENT, get_location(parser));
        ast->ident = accept_token(parser)->value;
    }
    else
    {
        // TODO: Throw error
        location_t loc = get_location(parser);
        parser->error = 1;
        console("[line %d, column %d] Expected expression, found '%s'.\n", loc.line, loc.column, current_token(parser)->value);
    }
    // TODO: Other matches
    return ast;
}

ast_t* parse_expression_last(parser_t* parser, ast_t* lhs, int minprec)
{
    /*while(1)
    {
        token_t* token = accept_token(parser);
        int prec = parse_precedence(token);
        return lhs;
    //    if(prec < minprec) return lhs;
}*/

    return lhs;
}

ast_t* parse_expression(parser_t* parser)
{
    ast_t* lhs = parse_expression_primary(parser);
    if(!lhs) return lhs;
    ast_t* rhs = parse_expression_last(parser, lhs, 0);
    assert(accept_token_type(parser, TOKEN_NEWLINE)->type == TOKEN_NEWLINE);
    return rhs;
}

ast_t* parse_stmt(parser_t* parser)
{
    static const struct
    {
        const char* token;
        ast_t* (*fn)(parser_t*);
    } parsers[] =
    {
        {KEYWORD_DECLARATION, parse_var_declaration}
    };

    size_t i;
    for(i = 0; i < sizeof(parsers)/sizeof(parsers[0]); i++)
    {
        if(match_string(parser, parsers[i].token))
        {
            return parsers[i].fn(parser);
        }
    }

    parser->error = 1;
    token_t* token = current_token(parser);
    location_t loc = token->location;
	console("[line %d, column %d] Could not interpret token '%s'\n", loc.line, loc.column, token->value);
    return 0;
}

ast_t* parser_run(parser_t* parser, const char* content)
{
    parser->buffer = lexer_lex(&parser->lexer, content, &parser->num_tokens);
    if(!parser->buffer) return 0;

    //nem_lexer_print_tokens(parser->buffer, parser->num_tokens);
    ast_t* ast = ast_class_create(AST_TOPLEVEL, get_location(parser));
    list_init(&ast->toplevel);

    while(!parser_end(parser))
    {
        skip_newline(parser);
        ast_t* node = parse_stmt(parser);
        if(!node || parser_error(parser))
        {
            ast_free(node);
            ast_free(ast);
            return 0;
        }
        list_push(&ast->toplevel, node);
    }

    return ast;
}

ast_t* parse_var_declaration(parser_t* parser)
{
    ast_t* node = ast_class_create(AST_DECLVAR, get_location(parser));

    // let x = expr \n
    token_t* var = accept_token_string(parser, KEYWORD_DECLARATION);
    token_t* ident = accept_token_type(parser, TOKEN_WORD);
    token_t* eq = accept_token_string(parser, "=");

    if(var && ident && eq)
    {
        node->vardecl.name = ident->value;
        node->vardecl.initializer = parse_expression(parser);
    }
    else
    {
        // TODO: Error handling
        free(node);
        return 0;
    }

    return node;
}
