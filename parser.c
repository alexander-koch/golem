#include "parser.h"

#define KEYWORD_DECLARATION "let"

ast_t* parse_var_declaration(parser_t* parser);
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
        case TOKEN_ASSIGN: return 0;
        case TOKEN_OR: return 1;
        case TOKEN_AND: return 2;
        case TOKEN_BITOR: return 3;
        case TOKEN_BITXOR: return 4;
        case TOKEN_BITAND: return 5;
        case TOKEN_EQUAL: return 6;
        case TOKEN_NEQUAL: return 6;
        case TOKEN_LESS: return 7;
        case TOKEN_GREATER: return 7;
        case TOKEN_LEQUAL: return 7;
        case TOKEN_GEQUAL: return 7;
        case TOKEN_BITLSHIFT: return 8;
        case TOKEN_BITRSHIFT: return 8;
        case TOKEN_SUB: return 9;
        case TOKEN_ADD: return 9;
        case TOKEN_MUL: return 9;
        case TOKEN_DIV: return 9;
        case TOKEN_MOD: return 9;
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
        console("Error: literal\n");
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

        assert(accept_token(parser)->type == TOKEN_RPAREN);
        // TODO: better error handling
    }
    else
    {
        // TODO: Throw error
        location_t loc = get_location(parser);
        parser->error = 1;
        console("[line %d, column %d] Expected expression, found '%s'.\n", loc.line, loc.column, current_token(parser)->value);
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
    ast_t* rhs = 0;
    for(;;)
    {
        /*if(match_type(parser, TOKEN_NEWLINE))
        {
            if(lhs->class != AST_BINARY)
            {
                ast_free(rhs);
            }
            return lhs;
        }*/

        token_type_t op = current_token(parser)->type;
        int prec = parse_precedence(op);
        if(prec < minprec)
        {
            /*if(lhs->class != AST_BINARY)
            {
                ast_free(rhs);
            }*/
            return lhs;
        }

        parser->cursor++;

        rhs = parse_expression_primary(parser);
        int nextprec = parse_precedence(current_token(parser)->type);
        if(prec < nextprec)
        {
        //    parser->cursor++;
            rhs = parse_expression_last(parser, rhs, prec + 1);
        }

        if(lhs->class == AST_NUMBER && rhs->class == AST_NUMBER)
        {
            switch(op)
            {
                case TOKEN_ADD:
                {
                    lhs->number += rhs->number;
                    continue;
                }
                case TOKEN_SUB:
                {
                    lhs->number -= rhs->number;
                    continue;
                }
                case TOKEN_MUL:
                {
                    lhs->number *= rhs->number;
                    continue;
                }
                case TOKEN_DIV:
                {
                    lhs->number /= rhs->number;
                    continue;
                }
                case TOKEN_MOD:
                {
                    lhs->number = (uint32_t)lhs->number % (uint32_t)rhs->number;
                    continue;
                }
                case TOKEN_BITAND:
                {
                    lhs->number = (uint32_t)lhs->number & (uint32_t)rhs->number;
                    continue;
                }
                case TOKEN_BITOR:
                {
                    lhs->number = (uint32_t)lhs->number | (uint32_t)rhs->number;
                    continue;
                }
                case TOKEN_BITLSHIFT:
                {
                    lhs->number = (uint32_t)lhs->number << (uint32_t)rhs->number;
                    continue;
                }
                case TOKEN_BITRSHIFT:
                {
                    lhs->number = (uint32_t)lhs->number >> (uint32_t)rhs->number;
                    continue;
                }
                case TOKEN_BITXOR:
                {
                    lhs->number = (uint32_t)lhs->number ^ (uint32_t)rhs->number;
                    continue;
                }
                case TOKEN_LESS:
                {
                    lhs->number = !!(lhs->number < rhs->number);
                    continue;
                }
                case TOKEN_GREATER:
                {
                    lhs->number = !!(lhs->number > rhs->number);
                    continue;
                }
                case TOKEN_LEQUAL:
                {
                    lhs->number = !!(lhs->number <= rhs->number);
                    continue;
                }
                case TOKEN_GEQUAL:
                {
                    lhs->number = !!(lhs->number >= rhs->number);
                    continue;
                }
                case TOKEN_EQUAL:
                {
                    lhs->number = lhs->number == rhs->number;
                    continue;
                }
                case TOKEN_NEQUAL:
                {
                    lhs->number = lhs->number != rhs->number;
                    continue;
                }
                default: break;
            }
        }

        ast_t* new_lhs = ast_class_create(AST_BINARY, get_location(parser));
        new_lhs->binary.left = lhs;
        new_lhs->binary.right = rhs;
        new_lhs->binary.op = op;
        lhs = new_lhs;

        console("Connected %s and %s to binary\n", ast_classname(lhs->class), ast_classname(rhs->class));
    }
}

ast_t* parse_expression(parser_t* parser)
{
    ast_t* lhs = parse_expression_primary(parser);
    if(!lhs) return lhs;
    ast_t* rhs = parse_expression_last(parser, lhs, 0);
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
            ast_t* node = parsers[i].fn(parser);
            assert(accept_token_type(parser, TOKEN_NEWLINE)->type == TOKEN_NEWLINE);
            return node;
        }
    }

    ast_t* node = parse_expression(parser);
    if(node)
    {
        assert(accept_token_type(parser, TOKEN_NEWLINE)->type == TOKEN_NEWLINE);
        return node;
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
