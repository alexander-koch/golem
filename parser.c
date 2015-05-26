#include "parser.h"

#define KEYWORD_DECLARATION "let"
#define KEYWORD_MUTATE "mod"
#define KEYWORD_FUNCTION "fn"

ast_t* parse_var_declaration(parser_t* parser);
ast_t* parse_fn_declaration(parser_t* parser);
ast_t* parse_expression(parser_t* parser);

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
        fprintf(stdout, "Wrong token type: %d. Expected: %d\n", parser->buffer[parser->cursor].type, type);
    }
    return 0;
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

void parser_throw(parser_t* parser, const char* format, ...)
{
    parser->error = 1;
    location_t loc = get_location(parser);

    fprintf(stdout, "[line %d, column %d] ", loc.line, loc.column);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
    fprintf(stdout, "\n");
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

int parse_precedence(token_type_t type)
{
    switch(type)
    {
        case TOKEN_ASSIGN: return 0;    // =
        case TOKEN_OR: return 1;        // ||
        case TOKEN_AND: return 2;       // &&
        case TOKEN_BITOR: return 3;     // |
        case TOKEN_BITXOR: return 4;    // ^
        case TOKEN_BITAND: return 5;    // &
        case TOKEN_EQUAL: return 6;     // ==
        case TOKEN_NEQUAL: return 6;    // !=
        case TOKEN_LESS: return 7;      // <
        case TOKEN_GREATER: return 7;   // >
        case TOKEN_LEQUAL: return 7;    // <=
        case TOKEN_GEQUAL: return 7;    // >=
        case TOKEN_BITLSHIFT: return 8; // <<
        case TOKEN_BITRSHIFT: return 8; // >>
        case TOKEN_SUB: return 9;       // -
        case TOKEN_ADD: return 9;       // +
        case TOKEN_MUL: return 10;      // *
        case TOKEN_DIV: return 10;      // /
        case TOKEN_MOD: return 10;      // %
        default: return -1;
    }
}

ast_t* parse_call(parser_t* parser, ast_t* node)
{
    ast_t* class = ast_class_create(AST_CALL, node->location);
    list_init(&class->call.args);
    class->call.callee = node;

    while(!match_type(parser, TOKEN_RPAREN))
    {
        list_push(&class->call.args, (void*)parse_expression(parser));
        if(match_type(parser, TOKEN_COMMA))
        {
            accept_token(parser);
        }
        else
        {
            break;
        }
    }

    token_t* token = accept_token_type(parser, TOKEN_RPAREN);
    assert(token);
    // TODO: Better error handling

    return class;
}

ast_t* parse_simpleliteral(parser_t* parser)
{
    ast_t* node = ast_class_create(-1, get_location(parser));
    if(match_type(parser, TOKEN_FLOAT))
    {
        node->class = AST_FLOAT;
        node->f = atof(accept_token(parser)->value);
        return node;
    }
    else if(match_type(parser, TOKEN_INT))
    {
        node->class = AST_INT;
        node->i = atol(accept_token(parser)->value);
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
        parser_throw(parser, "Token is not a literal");
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
    else if(match_type(parser, TOKEN_LPAREN))
    {
        accept_token(parser);
        ast = parse_expression(parser);
        // ast->class = AST_TUPLE;

        assert(accept_token(parser)->type == TOKEN_RPAREN);
        // TODO: better error handling
    }
    else
    {
        parser_throw(parser, "Expected expression, found '%s'", current_token(parser)->value);
    }
    // TODO: Other matches

    if(match_type(parser, TOKEN_LPAREN))
    {
        accept_token(parser);
        return parse_call(parser, ast);
    }

    return ast;
}

ast_t* parse_expression_last(parser_t* parser, ast_t* lhs, int minprec)
{
    for(;;)
    {
        token_type_t op = current_token(parser)->type;
        int prec = parse_precedence(op);
        if(prec < minprec)
        {
            return lhs;
        }

        parser->cursor++;

        // TODO: Test if next is a valid operator

        ast_t* rhs = parse_expression_primary(parser);
        int nextprec = parse_precedence(current_token(parser)->type);
        if(prec < nextprec)
        {
        //    parser->cursor++;
            rhs = parse_expression_last(parser, rhs, prec + 1);
            if(!rhs)
            {
                return 0;
            }
        }

        ast_t* new_lhs = ast_class_create(AST_BINARY, get_location(parser));
        new_lhs->binary.left = lhs;
        new_lhs->binary.right = rhs;
        new_lhs->binary.op = op;
        lhs = new_lhs;
    }
    return 0;
}

ast_t* parse_expression(parser_t* parser)
{
    ast_t* lhs = parse_expression_primary(parser);
    if(!lhs) return lhs;

    // TODO test if next is a valid operator
    if(!match_type(parser, TOKEN_NEWLINE))
    {
        ast_t* rhs = parse_expression_last(parser, lhs, 0);
        return rhs;
    }

    return lhs;
}

void test_newline(parser_t* parser)
{
    if(!parser->error)
    {
        // TODO: Create a function for throwing errors using vaargs
        // which does:
        // throw_error(msg)
        //  parser->error = 1
        //  console("[line %d, column %d] %s\n", msg);
        if(accept_token_type(parser, TOKEN_NEWLINE)->type != TOKEN_NEWLINE)
        {
            parser_throw(parser, "Missing newline after statement");
        }
    }
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
            ast_t* node = parsers[i].fn(parser);
            test_newline(parser);
            return node;
        }
    }

    ast_t* node = parse_expression(parser);
    if(node)
    {
        test_newline(parser);
        return node;
    }

    token_t* token = current_token(parser);
    parser_throw(parser, "Could not interpret token '%s'", token->value);
    return 0;
}

ast_t* parser_run(parser_t* parser, const char* content)
{
    parser->buffer = lexer_lex(&parser->lexer, content, &parser->num_tokens);
    if(!parser->buffer) return 0;

    // lexer_print_tokens(parser->buffer, parser->num_tokens);

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
    // let x = expr \n
    ast_t* node = ast_class_create(AST_DECLVAR, get_location(parser));
    bool mutate = false;

    token_t* var = accept_token_string(parser, KEYWORD_DECLARATION);
    if(match_string(parser, KEYWORD_MUTATE))
    {
        accept_token(parser);
        mutate = true;
    }

    token_t* ident = accept_token_type(parser, TOKEN_WORD);
    token_t* eq = accept_token_string(parser, "=");

    if(var && ident && eq)
    {
        node->vardecl.name = ident->value;
        node->vardecl.mutate = mutate;
        node->vardecl.initializer = parse_expression(parser);
    }
    else
    {
        parser_throw(parser, "Malformed variable declaration");
        node->class = AST_NULL;
    }

    return node;
}

ast_t* parse_fn_declaration(parser_t* parser)
{
    // fn name (params) { \n
    //ast_t* node = ast_class_create(AST_LAMBDA, get_location(parser));

    return 0;
}
