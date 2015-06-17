#include "parser.h"

#define KEYWORD_INCLUDE "use"
#define KEYWORD_DECLARATION "let"
#define KEYWORD_MUTATE "mut"
#define KEYWORD_FUNCTION "func"
#define KEYWORD_IF "if"
#define KEYWORD_ELSE "else"
#define KEYWORD_WHILE "while"
#define KEYWORD_CLASS "class"
#define KEYWORD_RETURN "return"

ast_t* parse_include_declaration(parser_t* parser);
ast_t* parse_var_declaration(parser_t* parser);
ast_t* parse_fn_declaration(parser_t* parser);
ast_t* parse_if_declaration(parser_t* parser);
ast_t* parse_while_declaration(parser_t* parser);
ast_t* parse_class_declaration(parser_t* parser);
ast_t* parse_return_declaration(parser_t* parser);
ast_t* parse_expression(parser_t* parser);

ast_t* parse_stmt(parser_t* parser);
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

bool match_string(parser_t* parser, const char* token)
{
    if(parser_end(parser) || !token) return false;
    if(!parser->buffer[parser->cursor].value) return false;
    return !strcmp(parser->buffer[parser->cursor].value, token);
}

bool match_type(parser_t* parser, token_type_t type)
{
    return parser->buffer[parser->cursor].type == type;
}

bool match_next(parser_t* parser, const char* token)
{
    if(parser_end(parser) || parser->cursor+1 >= parser->num_tokens) return false;
    return !strcmp(parser->buffer[parser->cursor+1].value, token);
}

token_t* accept_token(parser_t* parser)
{
    return &parser->buffer[parser->cursor++];
}

token_t* accept_token_string(parser_t* parser, const char* str)
{
    if(parser_end(parser)) return 0;

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
        token_type_t tp = parser->buffer[parser->cursor].type;
        parser_throw(parser, "Invalid syntax token '%s'. Expected '%s'", tok2str(tp), tok2str(type));
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

    fprintf(stdout, "[line %d, column %d] (Syntax): ", loc.line, loc.column);
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
        match_type(parser, TOKEN_STRING) ||
        match_type(parser, TOKEN_BOOL);
}

int match_literal(parser_t* parser)
{
    return match_simple(parser) ||
        match_string(parser, "[");
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

/**
 *  Parses a call structure
 */
ast_t* parse_call(parser_t* parser, ast_t* node)
{
    ast_t* class = ast_class_create(AST_CALL, node->location);
    class->call.args = list_new();
    class->call.callee = node;
    ast_t* expr = 0;

    while(!match_type(parser, TOKEN_RPAREN) && (expr = parse_expression(parser)))
    {
        list_push(class->call.args, (void*)expr);
        if(match_type(parser, TOKEN_COMMA))
        {
            accept_token(parser);
        }
        else
        {
            // TODO: do better handling
            // FIXME: is it even an error, can it occur ?
            break;
        }
    }

    if(!match_type(parser, TOKEN_RPAREN))
    {
        parser_throw(parser, "Expected closing parenthesis");
    }
    else
    {
        accept_token(parser);
    }

    return class;
}

/**
 *  Parses a subscript, bracket based myarray[5] = x
 */
ast_t* parse_subscript(parser_t* parser, ast_t* node)
{
    ast_t* ast = ast_class_create(AST_SUBSCRIPT, node->location);
    ast->subscript.expr = node;
    ast->subscript.key = parse_expression(parser);

    if(!match_type(parser, TOKEN_RBRACKET))
    {
        parser_throw(parser, "Expected closing bracket");
        return ast;
    }

    accept_token(parser);
    if(match_type(parser, TOKEN_LBRACKET))
    {
        accept_token(parser);
        return parse_subscript(parser, ast);
    }

    return ast;
}

/**
 *  Parses a subscript, dot based. Example: myclass.x = 5
 */
ast_t* parse_subscript_sugar(parser_t* parser, ast_t* node)
{
    ast_t* ast = ast_class_create(AST_SUBSCRIPT, node->location);
    ast->subscript.expr = node;
    if(!match_type(parser, TOKEN_WORD))
    {
        parser_throw(parser, "Subscript: Identifier expected");
        return ast;
    }

    token_t* ident = accept_token(parser);
    ast_t* key = ast_class_create(AST_IDENT, ident->location);
    key->ident = ident->value;
    ast->subscript.key = key;

    if(match_type(parser, TOKEN_LPAREN))
    {
        accept_token(parser);
        return parse_call(parser, ast);
    }
    else if(match_type(parser, TOKEN_LBRACKET))
    {
        accept_token(parser);
        return parse_subscript(parser, ast);
    }
    else if(match_type(parser, TOKEN_DOT))
    {
        accept_token(parser);
        return parse_subscript_sugar(parser, ast);
    }
    return ast;
}

/**
 *  Parses simple literal structures -> Float / Integer / String / Boolean
 */
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
    else if(match_type(parser, TOKEN_BOOL))
    {
        node->class = AST_BOOL;
        node->b = !strcmp(accept_token(parser)->value, "true") ? true : false;
        return node;
    }
    else
    {
        parser_throw(parser, "Token is not a literal");
    }
    return node;
}

/**
 *  Array parsing function
 */
ast_t* parse_array(parser_t* parser)
{
    ast_t* ast = ast_class_create(AST_ARRAY, get_location(parser));
    ast->array = list_new();

    token_t* tmp = accept_token_type(parser, TOKEN_LBRACKET);
    if(!tmp)
    {
        parser_throw(parser, "Expected array begin");
    }

    ast_t* expr = 0;
    while(!match_type(parser, TOKEN_RBRACKET) && (expr = parse_expression(parser)))
    {
        list_push(ast->array, (void*)expr);
        if(!match_type(parser, TOKEN_COMMA))
        {
            break;
        }
        else
        {
            accept_token(parser);
        }
    }
    tmp = accept_token_type(parser, TOKEN_RBRACKET);
    if(!tmp)
    {
        parser_throw(parser, "Expected array end");
    }
    return ast;
}

/**
 *  Parses a literal
 *  A literal is either a number / string / boolean or an array
 */
ast_t* parse_literal(parser_t* parser)
{
    if(match_simple(parser))
    {
        return parse_simpleliteral(parser);
    }
    else if(match_type(parser, TOKEN_LBRACKET))
    {
        return parse_array(parser);
    }
    else
    {
        parser_throw(parser, "Could not parse literal");
    }
    return 0;
}

ast_t* parse_expression_primary(parser_t* parser)
{
    if(match_type(parser, TOKEN_NEWLINE))
    {
        return 0;
    }

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

        if(!match_type(parser, TOKEN_RPAREN))
        {
            parser_throw(parser, "Expected closing parenthesis");
        }
        else
        {
            accept_token(parser);
        }
    }
    else if(match_type(parser, TOKEN_ADD) || match_type(parser, TOKEN_SUB) ||
        match_type(parser, TOKEN_NOT) || match_type(parser, TOKEN_BITNOT) ||
        match_type(parser, TOKEN_MUL))
    {
        // +2 -> unary(+, 2)
        // -2 -> unary(-, 2)
        // !true -> unary(!, true)
        // ~2 -> unary(~, 2)
        // *x -> unary(*, x) -> pointer to something / value of
        // -2 + 3 -> bin(unary(-, 2), 3)
        // -(2 + 3) -> unary(-, bin(2,3))

        // TODO: How about prefix increment and decrement?

        // Prefix operator
        token_type_t op = accept_token(parser)->type;
        ast = ast_class_create(AST_UNARY, get_location(parser));
        ast->unary.op = op;
        ast->unary.expr = parse_expression_primary(parser);
    }
    else
    {
        parser_throw(parser, "Expected expression, found '%s'", current_token(parser)->value);
    }

    if(match_type(parser, TOKEN_LPAREN))
    {
        accept_token(parser);
        return parse_call(parser, ast);
    }
    else if(match_type(parser, TOKEN_LBRACKET))
    {
        accept_token(parser);
        return parse_subscript(parser, ast);
    }
    else if(match_type(parser, TOKEN_DOT))
    {
        accept_token(parser);
        return parse_subscript_sugar(parser, ast);
    }

    return ast;
}

ast_t* parse_expression_last(parser_t* parser, ast_t* lhs, int minprec)
{
    for(;;)
    {
        // TODO: Test if the operator is valid or not, otherwise
        // throws error of missing newline

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
            rhs = parse_expression_last(parser, rhs, prec + 1);
            if(!rhs)
            {
                return 0;
            }
        }

        // Optimize syntax tree
        // Integer optimization
        if(lhs->class == AST_INT && rhs->class == AST_INT)
        {
            bool opt = true;
            switch(op)
            {
                case TOKEN_ADD:
                {
                    lhs->i = lhs->i + rhs->i;
                    break;
                }
                case TOKEN_SUB:
                {
                    lhs->i = lhs->i - rhs->i;
                    break;
                }
                case TOKEN_MUL:
                {
                    lhs->i = lhs->i * rhs->i;
                    break;
                }
                case TOKEN_DIV:
                {
                    lhs->i = lhs->i / rhs->i;
                    break;
                }
                case TOKEN_MOD:
                {
                    lhs->i = lhs->i % rhs->i;
                    break;
                }
                case TOKEN_BITLSHIFT:
                {
                    lhs->i = lhs->i << rhs->i;
                    break;
                }
                case TOKEN_BITRSHIFT:
                {
                    lhs->i = lhs->i >> rhs->i;
                    break;
                }
                case TOKEN_BITAND:
                {
                    lhs->i = lhs->i & rhs->i;
                    break;
                }
                case TOKEN_BITOR:
                {
                    lhs->i = lhs->i | rhs->i;
                    break;
                }
                case TOKEN_BITXOR:
                {
                    lhs->i = lhs->i ^ rhs->i;
                    break;
                }
                case TOKEN_EQUAL:
                {
                    lhs->class = AST_BOOL;
                    lhs->b = (lhs->i == rhs->i);
                    break;
                }
                case TOKEN_NEQUAL:
                {
                    lhs->class = AST_BOOL;
                    lhs->b = (lhs->i != rhs->i);
                    break;
                }
                case TOKEN_LEQUAL:
                {
                    lhs->class = AST_BOOL;
                    lhs->b = (lhs->i <= lhs->i);
                    break;
                }
                case TOKEN_GEQUAL:
                {
                    lhs->class = AST_BOOL;
                    lhs->b = (lhs->i >= lhs->i);
                    break;
                }
                case TOKEN_LESS:
                {
                    lhs->class = AST_BOOL;
                    lhs->b = (lhs->i < lhs->i);
                    break;
                }
                case TOKEN_GREATER:
                {
                    lhs->class = AST_BOOL;
                    lhs->b = (lhs->i > lhs->i);
                    break;
                }
                default:
                {
                    opt = false;
                    break;
                }
            }

            if(opt)
            {
                ast_free(rhs);
                continue;
            }
        }

        // Float optimization
        if(lhs->class == AST_FLOAT && rhs->class == AST_INT)
        {
            // convert right hand side to float too
            rhs->class = AST_FLOAT;
            rhs->f = (F64)rhs->i;
        }
        else if(lhs->class == AST_INT && rhs->class == AST_FLOAT)
        {
            lhs->class = AST_FLOAT;
            lhs->f = (F64)lhs->i;
        }

        ast_t* new_lhs = ast_class_create(AST_BINARY, get_location(parser));
        new_lhs->binary.left = lhs;
        new_lhs->binary.right = rhs;
        new_lhs->binary.op = op;
        lhs = new_lhs;
    }
    return 0;
}

/**
 *  Parses an expression using two subroutines
 */
ast_t* parse_expression(parser_t* parser)
{
    ast_t* lhs = parse_expression_primary(parser);
    if(!lhs) return lhs;

    // TODO: test if next is a valid operator
    if(!match_type(parser, TOKEN_NEWLINE))
    {
        ast_t* rhs = parse_expression_last(parser, lhs, 0);
        return rhs;
    }

    return lhs;
}

/**
 *  Parse formals -> e.g. argument list for functions
 */
list_t* parse_formals(parser_t* parser)
{
    list_t* formals = list_new();
    while(!match_type(parser, TOKEN_RPAREN))
    {
        if(!match_type(parser, TOKEN_WORD))
        {
            parser_throw(parser, "Invalid argument list");
            return formals;
        }

        list_push(formals, accept_token(parser)->value);
        if(!match_type(parser, TOKEN_COMMA) && !match_type(parser, TOKEN_RPAREN))
        {
            parser_throw(parser, "Expected seperator");
            return formals;
        }
    }

    token_t* rparen = accept_token_type(parser, TOKEN_RPAREN);
    if(!rparen)
    {
        parser_throw(parser, "Missing right parenthesis");
    }

    return formals;
}

/**
 *  Parses block until end
 *  DOES NOT PARSE BEGIN WITH AN OPENING BRACE
 */
list_t* parse_block(parser_t* parser)
{
    list_t* statements = list_new();
    skip_newline(parser);
    while(!match_type(parser, TOKEN_RBRACE) && !parser_error(parser))
    {
        if(parser_end(parser))
        {
            parser_throw(parser, "Block not closed, reached end");
            return statements;
        }

        list_push(statements, (void*)parse_stmt(parser));
        skip_newline(parser);
    }

    if(!parser_error(parser))
    {
        token_t* rbrace = accept_token_type(parser, TOKEN_RBRACE);
        if(!rbrace)
        {
            parser_throw(parser, "Missing closing brace");
        }
    }

    return statements;
}

/**
 *  Newline testing, throws error if not
 */
void test_newline(parser_t* parser)
{
    if(!parser->error)
    {
        if(!match_type(parser, TOKEN_NEWLINE))
        {
            parser_throw(parser, "Invalid statement (Newline missing?)");
        }
        else
        {
            accept_token(parser);
        }
    }
}

/**
 *  Statement parsing
 *  Every statment is followed by a newline
 */
ast_t* parse_stmt(parser_t* parser)
{
    static const struct
    {
        const char* token;
        ast_t* (*fn)(parser_t*);
    } parsers[] =
    {
        {KEYWORD_INCLUDE, parse_include_declaration},
        {KEYWORD_DECLARATION, parse_var_declaration},
        {KEYWORD_FUNCTION, parse_fn_declaration},
        {KEYWORD_IF, parse_if_declaration},
        {KEYWORD_WHILE, parse_while_declaration},
        {KEYWORD_CLASS, parse_class_declaration},
        {KEYWORD_RETURN, parse_return_declaration}
    };

    for(size_t i = 0; i < sizeof(parsers)/sizeof(parsers[0]); i++)
    {
        if(match_string(parser, parsers[i].token))
        {
            ast_t* node = parsers[i].fn(parser);
            test_newline(parser);
            return node;
        }
    }

    // Error testing
    if(match_string(parser, KEYWORD_ELSE))
    {
        parser_throw(parser, "If-clause error: No beginning if-statement / Else-if out of if-chain");
        return 0;
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

///////-----------------
// Main parsing function
//////------------------

ast_t* parser_run(parser_t* parser, const char* content)
{
    parser->buffer = lexer_lex(&parser->lexer, content, &parser->num_tokens);
    if(!parser->buffer) return 0;

    // Use this for lexical analysis, debug if tokens are read wrong
    // lexer_print_tokens(parser->buffer, parser->num_tokens);

    // Create toplevel program scope
    ast_t* ast = ast_class_create(AST_TOPLEVEL, get_location(parser));
    ast->toplevel = list_new();

    // Parse each statement and abort if an error occurs
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

        // add node to program
        list_push(ast->toplevel, node);
    }

    bool b = optimize_tree(ast);
    if(!b)
    {
        ast_free(ast);
        return 0;
    }

    return ast;
}

///////-----------------
// Parsing subroutines
//////------------------

ast_t* parse_include_declaration(parser_t* parser)
{
    // use "io" \n
    ast_t* node = ast_class_create(AST_INCLUDE, get_location(parser));
    token_t* inc = accept_token_string(parser, KEYWORD_INCLUDE);
    token_t* val = accept_token_type(parser, TOKEN_STRING);

    if(inc && val)
    {
        ast_t* str = ast_class_create(AST_STRING, get_location(parser));
        str->string = val->value;
        node->include = str;
    }
    else
    {
        parser_throw(parser, "Malformed import / include statement");
    }

    return node;
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
    token_t* eq = accept_token_type(parser, TOKEN_ASSIGN);

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
    ast_t* node = ast_class_create(AST_DECLFUNC, get_location(parser));

    token_t* fn = accept_token_string(parser, KEYWORD_FUNCTION);
    token_t* ident = accept_token_type(parser, TOKEN_WORD);
    token_t* lparen = accept_token_type(parser, TOKEN_LPAREN);

    if(fn && ident && lparen)
    {
        node->funcdecl.name = ident->value;
        node->funcdecl.impl.formals = parse_formals(parser);

        if(!match_type(parser, TOKEN_LBRACE))
        {
            parser_throw(parser, "Block begin expected");
        }
        else
        {
            accept_token(parser);
            accept_token_type(parser, TOKEN_NEWLINE);
            node->funcdecl.impl.body = parse_block(parser);
        }
    }
    else
    {
        parser_throw(parser, "Malformed function declaration");
    }

    return node;
}

ast_t* parse_if_declaration(parser_t* parser)
{
    // if (expr) { \n
    ast_t* node = ast_class_create(AST_IF, get_location(parser));
    node->ifstmt = list_new();

    // if (if / else if) do
    while(match_string(parser, KEYWORD_IF) || (match_string(parser, KEYWORD_ELSE) && match_next(parser, KEYWORD_IF)))
    {
        // create a subclause and skip tokens
        ast_t* clause = ast_class_create(AST_IFCLAUSE, get_location(parser));
        if(match_string(parser, KEYWORD_IF))
        {
            accept_token_string(parser, KEYWORD_IF);
        }
        else
        {
            accept_token_string(parser, KEYWORD_ELSE);
            accept_token_string(parser, KEYWORD_IF);
        }

        // begin expression
        token_t* lparen = accept_token_type(parser, TOKEN_LPAREN);
        if(!lparen)
        {
            parser_throw(parser, "Malformed if-statement, opening parenthesis expected");
            ast_free(clause);
            return node;
        }
        else
        {
            clause->ifclause.cond = parse_expression(parser);
        }

        token_t* rparen = accept_token_type(parser, TOKEN_RPAREN);
        token_t* lbrace = accept_token_type(parser, TOKEN_LBRACE);

        if(rparen && lbrace)
        {
            clause->ifclause.body = parse_block(parser);
        }
        else
        {
            parser_throw(parser, "Conditional expression without end");
            ast_free(clause);
            return node;
        }

        list_push(node->ifstmt, clause);
    }

    if(match_string(parser, KEYWORD_ELSE))
    {
        ast_t* clause = ast_class_create(AST_IFCLAUSE, get_location(parser));
        token_t* elsetok = accept_token_string(parser, KEYWORD_ELSE);
        token_t* lbrace = accept_token_type(parser, TOKEN_LBRACE);

        if(elsetok && lbrace)
        {
            clause->ifclause.cond = 0;
            clause->ifclause.body = parse_block(parser);
        }
        else
        {
            parser_throw(parser, "Else statement without else keyword");
            free(clause);
            return node;
        }

        list_push(node->ifstmt, clause);
    }

    return node;
}

ast_t* parse_while_declaration(parser_t* parser)
{
    // while (expr) { \n
    ast_t* node = ast_class_create(AST_WHILE, get_location(parser));

    token_t* key = accept_token_string(parser, KEYWORD_WHILE);
    token_t* lparen = accept_token_type(parser, TOKEN_LPAREN);

    if(key && lparen)
    {
        node->whilestmt.cond = parse_expression(parser);

        token_t* rparen = accept_token_type(parser, TOKEN_RPAREN);
        token_t* lbrace = accept_token_type(parser, TOKEN_LBRACE);

        if(rparen && lbrace)
        {
            node->whilestmt.body = parse_block(parser);
        }
        else
        {
            parser_throw(parser, "While loop without end");
        }
    }
    else
    {
        parser_throw(parser, "Malformed while loop block");
    }

    return node;
}

ast_t* parse_class_declaration(parser_t* parser)
{
    // class expr { \n
    ast_t* node = ast_class_create(AST_CLASS, get_location(parser));

    token_t* key = accept_token_string(parser, KEYWORD_CLASS);
    token_t* ident = accept_token_type(parser, TOKEN_WORD);
    token_t* lbrace = accept_token_type(parser, TOKEN_LBRACE);

    if(key && ident && lbrace)
    {
        node->classstmt.name = ident->value;
        node->classstmt.body = parse_block(parser);
    }
    else
    {
        parser_throw(parser, "Malformed class declaration");
    }

    return node;
}

ast_t* parse_return_declaration(parser_t* parser)
{
    ast_t* node = ast_class_create(AST_RETURN, get_location(parser));
    token_t* ret = accept_token_string(parser, KEYWORD_RETURN);

    if(ret)
    {
        node->returnstmt = parse_expression(parser);
    }
    else
    {
        parser_throw(parser, "Invalid return statement");
    }

    return node;
}
