#include "parser.h"

// Forward declaration of main parsing methods
ast_t* parse_import_declaration(parser_t* parser, location_t loc);
ast_t* parse_var_declaration(parser_t* parser, location_t loc);
ast_t* parse_fn_declaration(parser_t* parser, location_t loc);
ast_t* parse_if_declaration(parser_t* parser, location_t loc);
ast_t* parse_while_declaration(parser_t* parser, location_t loc);
ast_t* parse_class_declaration(parser_t* parser, location_t loc);
ast_t* parse_return_declaration(parser_t* parser, location_t loc);
ast_t* parse_annotation_declaration(parser_t* parser, location_t loc);
ast_t* parse_expression(parser_t* parser);
ast_t* parse_stmt(parser_t* parser);
ast_t* parse_expression(parser_t* parser);
void test_newline(parser_t* parser);

void parser_init(parser_t* parser, context_t* context, const char* name) {
    parser->name = name;
    parser->buffer = 0;
    parser->num_tokens = 0;
    parser->cursor = 0;
    parser->error = false;
    parser->top = 0;
    parser->context = context;
}

// helper functions begin

// Check if we reached the end
int parser_end(parser_t* parser) {
    return parser->cursor >= parser->num_tokens;
}

// Get the token under the cursor
token_t* current_token(parser_t* parser) {
    return &parser->buffer[parser->cursor];
}

// Get the location in code of the current cursor
location_t get_location(parser_t* parser) {
    return current_token(parser)->location;
}

// Test if the current token matches a given type
bool match_type(parser_t* parser, token_type_t type) {
    return current_token(parser)->type == type;
}

bool match_next(parser_t* parser, token_type_t type) {
    if(parser_end(parser) || parser->cursor+1 >= parser->num_tokens) return false;
    return parser->buffer[parser->cursor+1].type == type;
}

token_t* accept_token(parser_t* parser) {
    return &parser->buffer[parser->cursor++];
}

token_t* accept_token_type(parser_t* parser, token_type_t type) {
    if(parser_end(parser)) return 0;

    if(parser->buffer[parser->cursor].type == type) {
        return accept_token(parser);
    } else {
        token_type_t tp = parser->buffer[parser->cursor].type;
        parser_throw(parser, "Invalid syntax token '%s'. Expected '%s'",
            token_string(tp), token_string(type));
    }
    return 0;
}

void skip_newline(parser_t* parser) {
    while(parser->buffer[parser->cursor].type == TOKEN_NEWLINE) {
        parser->cursor++;
    }
}

void parser_free(parser_t* parser) {
    ast_free(parser->top);
    lexer_free_buffer(parser->buffer, parser->num_tokens);
}

int parser_error(parser_t* parser) {
    return parser->error;
}

/**
 * parser_throw:
 * If an error occurs, use this function to print the error.
 * Requires the parser to print the location.
 */
void parser_throw(parser_t* parser, const char* format, ...) {
    parser->error = 1;
    location_t loc = get_location(parser);

    printf("%s:%d:%d (Syntax): ", parser->name, loc.line, loc.column);
    va_list argptr;
    va_start(argptr, format);
    vprintf(format, argptr);
    va_end(argptr);
    putchar('\n');
}

// Parsing Functions
int match_simple(parser_t* parser) {
    return match_type(parser, TOKEN_INT)
        || match_type(parser, TOKEN_FLOAT)
        || match_type(parser, TOKEN_STRING)
        || match_type(parser, TOKEN_BOOL)
        || match_type(parser, TOKEN_NONE);
}

int match_literal(parser_t* parser) {
    return match_simple(parser)
        || match_type(parser, TOKEN_LBRACKET);
}

/**
 * parse_precedence:
 * Returns the precedence for given token_type_t type.
 * If the token type is not listed, -1 is returned.
 */
int parse_precedence(token_type_t type) {
    switch(type) {
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

ast_t* parse_subscript_sugar(parser_t* parser, ast_t* node);
ast_t* parse_subscript(parser_t* parser, ast_t* node);
datatype_t* parse_datatype(parser_t* parser);

/**
 * parse_call:
 * Parses a call structure
 * Examples: func(), foo(baz, bar)
 *
 * Call:
 *  Args:List
 *  Callee:Node
 *
 * EBNF:
 * Call = Expression "(" [ Expression { "," Expression } ] ")" .
 */
ast_t* parse_call(parser_t* parser, ast_t* node) {
    ast_t* class = ast_class_create(AST_CALL, node->location);
    class->call.args = list_new();
    class->call.callee = node;
    ast_t* expr = 0;

    if(node->class != AST_IDENT && node->class != AST_SUBSCRIPT) {
        parser_throw(parser, "Function callee has to be an identifier");
        return class;
    }

    while(!match_type(parser, TOKEN_RPAREN) && (expr = parse_expression(parser))) {
        list_push(class->call.args, expr);
        if(match_type(parser, TOKEN_COMMA)) {
            parser->cursor++;
        } else {
            break;
        }
    }

    if(!match_type(parser, TOKEN_RPAREN)) {
        parser_throw(parser, "Expected closing parenthesis");
    } else {
        parser->cursor++;
    }

    if(match_type(parser, TOKEN_DOT)) {
        parser->cursor++;
        return parse_subscript_sugar(parser, class);
    } else if(match_type(parser, TOKEN_LBRACKET)) {
        parser->cursor++;
        return parse_subscript(parser, class);
    }

    return class;
}

/**
 * parse_subscript:
 * Parses a subscript (bracket based).
 * Example: myarray[5] = x
 *
 * Subscript:
 *  Expr:Node
 *  Key:Expression
 *
 * EBNF:
 * Subscript = Expression "[" Expression "]" .
 */
ast_t* parse_subscript(parser_t* parser, ast_t* node) {
    // Create the node
    ast_t* ast = ast_class_create(AST_SUBSCRIPT, node->location);
    ast->subscript.expr = node;
    ast->subscript.key = parse_expression(parser);

    if(!match_type(parser, TOKEN_RBRACKET)) {
        parser_throw(parser, "Expected closing bracket");
        return ast;
    }

    // Consume the bracket
    parser->cursor++;

    // Continue the parsing if possible
    if(match_type(parser, TOKEN_LPAREN)) {
        parser->cursor++;
        return parse_call(parser, ast);
    } else if(match_type(parser, TOKEN_LBRACKET)) {
        parser->cursor++;
        return parse_subscript(parser, ast);
    } else if(match_type(parser, TOKEN_DOT)) {
        parser->cursor++;
        return parse_subscript_sugar(parser, ast);
    }

    return ast;
}

/**
 * parse_subscript_sugar:
 * Parses a special type of a subscript (syntactic sugar).
 * The subscript uses a dot instead of brackets.
 * Example: myclass.x = 5
 *
 * Subscript:
 *  Expr:Node
 *  Key:Ident
 *
 * EBNF:
 * Subscript_sugar = Expression "." Ident .
 */
ast_t* parse_subscript_sugar(parser_t* parser, ast_t* node) {
    // Create the subscript node
    ast_t* ast = ast_class_create(AST_SUBSCRIPT, node->location);
    ast->subscript.expr = node;

    if(!match_type(parser, TOKEN_WORD)) {
        parser_throw(parser, "Subscript: Identifier expected");
        return ast;
    }

    // Consume the identifier
    token_t* ident = accept_token(parser);

    // Create the identifier node and set is as key
    ast_t* key = ast_class_create(AST_IDENT, ident->location);
    key->ident = ident->value;
    ast->subscript.key = key;

    if(match_type(parser, TOKEN_LPAREN)) {
        parser->cursor++;
        return parse_call(parser, ast);
    } else if(match_type(parser, TOKEN_LBRACKET)) {
        parser->cursor++;
        return parse_subscript(parser, ast);
    } else if(match_type(parser, TOKEN_DOT)) {
        parser->cursor++;
        return parse_subscript_sugar(parser, ast);
    }
    return ast;
}

/**
 * parse_simpleliteral:
 *
 * EBNF:
 * SimpleLiteral = Float | Integer | String | Boolean | None .
 */
ast_t* parse_simpleliteral(parser_t* parser) {
    // Create a 'placeholder'-node
    ast_t* node = ast_class_create(-1, get_location(parser));

    if(match_type(parser, TOKEN_FLOAT)) {
        node->class = AST_FLOAT;
        node->f = atof(accept_token(parser)->value);
    } else if(match_type(parser, TOKEN_INT)) {
        node->class = AST_INT;
        node->i = atol(accept_token(parser)->value);
    } else if(match_type(parser, TOKEN_STRING)) {
        node->class = AST_STRING;
        node->string = accept_token(parser)->value;

        if(strlen(node->string) == 1) {
            node->class = AST_CHAR;
            node->ch = node->string[0];
        }
    } else if(match_type(parser, TOKEN_BOOL)) {
        node->class = AST_BOOL;
        node->b = !strcmp(accept_token(parser)->value, "true") ? true : false;
    } else if(match_type(parser, TOKEN_NONE)) {
        parser->cursor++;
        node->class = AST_NONE;
        token_t* lbracket = accept_token_type(parser, TOKEN_LESS);
        if(!lbracket) {
            parser_throw(parser, "Expected an opening bracket (<)");
            return node;
        }

        node->none.type = parse_datatype(parser);
        token_t* rbracket = accept_token_type(parser, TOKEN_GREATER);
        if(!rbracket) {
            parser_throw(parser, "Expected an opening bracket (>)");
            return node;
        }
    } else {
        parser_throw(parser, "Token is not a literal");
    }
    return node;
}

/**
 * parse_array:
 * Array parsing function.
 *
 * Example:
 * let x = [1,2,3,4,5]
 * let y = [::int]
 *
 * EBNF:
 * Array = "[", ( ( Expression { "," Expression } ) | ( "::" Datatype ) ) "]" .
 */
ast_t* parse_array(parser_t* parser) {
    ast_t* ast = ast_class_create(AST_ARRAY, get_location(parser));
    ast->array.elements = list_new();
    ast->array.type = context_null(parser->context);

    token_t* tmp = accept_token_type(parser, TOKEN_LBRACKET);
    if(!tmp) {
        parser_throw(parser, "Expected array begin");
        // TODO: Return ast?
    }

    skip_newline(parser);

    // New Feature 'Doublecolon initializer': [::int] -> initializes array with zero elements of type int
    if(match_type(parser, TOKEN_DOUBLECOLON)) {
        accept_token(parser);
        datatype_t* dt = parse_datatype(parser);
        ast->array.type = dt;

        if(!match_type(parser, TOKEN_RBRACKET)) {
            parser_throw(parser, "Expected closing bracket");
        } else {
            accept_token(parser);
        }
        return ast;
    }

    if(match_type(parser, TOKEN_RBRACKET)) {
        parser_throw(parser, "Initialized array with no elements");
        return ast;
    }

    ast_t* expr = 0;
    while(!match_type(parser, TOKEN_RBRACKET) && (expr = parse_expression(parser))) {
        list_push(ast->array.elements, (void*)expr);
        skip_newline(parser);
        if(!match_type(parser, TOKEN_COMMA)) {
            break;
        } else {
            parser->cursor++;
        }
        skip_newline(parser);
    }
    skip_newline(parser);
    tmp = accept_token_type(parser, TOKEN_RBRACKET);
    if(!tmp) {
        parser_throw(parser, "Expected array end");
    }
    return ast;
}

/**
 * parse_literal:
 *
 * EBNF:
 * Literal = SimpleLiteral | Array .
 */
ast_t* parse_literal(parser_t* parser) {
    if(match_simple(parser)) {
        return parse_simpleliteral(parser);
    } else if(match_type(parser, TOKEN_LBRACKET)) {
        return parse_array(parser);
    } else {
        parser_throw(parser, "Could not parse literal");
    }
    return 0;
}

/**
 *
 * parse_expression_primary:
 * Parses a primary expression.
 *
 * EBNF:
 * Expression_primary =
 * Literal | TOKEN_WORD | ( "(" Expression ")" ) | Unary
 * | Call | Subscript | Subscript_sugar .
 */
ast_t* parse_expression_primary(parser_t* parser) {
    if(match_type(parser, TOKEN_NEWLINE)) {
        return 0;
    }

    ast_t* ast = 0;
    // Literal
    if(match_literal(parser)) {
        // Example literals: 1|2|3|4|5 - "Hello World", true, false
        ast = parse_literal(parser);
    }
    // Identifier
    else if(match_type(parser, TOKEN_WORD)) {
        // myscript.access = denied
        ast = ast_class_create(AST_IDENT, get_location(parser));
        ast->ident = accept_token(parser)->value;
    }
    // (expr)
    else if(match_type(parser, TOKEN_LPAREN)) {
        // (2 + 3) * 5
        accept_token(parser);
        ast = parse_expression(parser);

        if(!match_type(parser, TOKEN_RPAREN)) {
            parser_throw(parser, "Expected closing parenthesis");
        } else {
            accept_token(parser);
        }
    }
    // Unary
    else if(match_type(parser, TOKEN_ADD) || match_type(parser, TOKEN_SUB)
        || match_type(parser, TOKEN_NOT) || match_type(parser, TOKEN_BITNOT)) {
        // Binary operators: plus, minus, not, bitnot

        // +2 -> unary(+, 2)
        // -2 -> unary(-, 2)
        // !true -> unary(!, true)
        // ~2 -> unary(~, 2)
        // -2 + 3 -> bin(unary(-, 2), 3)
        // -(2 + 3) -> unary(-, bin(2,3))

        // TODO: How about prefix increment and decrement?

        // Prefix operator
        token_type_t op = accept_token(parser)->type;
        ast = ast_class_create(AST_UNARY, get_location(parser));
        ast->unary.op = op;
        ast->unary.expr = parse_expression_primary(parser);
    }
    // Error
    else {
        parser_throw(parser, "Expected expression, found '%s'", current_token(parser)->value);
        return ast;
    }

    // If no error occured, try to continue.
    if(match_type(parser, TOKEN_LPAREN)) {
        accept_token(parser);
        return parse_call(parser, ast);
    }
    else if(match_type(parser, TOKEN_LBRACKET)) {
        accept_token(parser);
        return parse_subscript(parser, ast);
    }
    else if(match_type(parser, TOKEN_DOT)) {
        accept_token(parser);
        return parse_subscript_sugar(parser, ast);
    }

    return ast;
}

ast_t* parse_expression_last(parser_t* parser, ast_t* lhs, int minprec) {
    for(;;) {
        // TODO: Test if the operator is valid or not, otherwise
        // throws error of missing newline

        token_type_t op = current_token(parser)->type;
        int prec = parse_precedence(op);
        if(prec < minprec) {
            return lhs;
        }

        parser->cursor++;

        // TODO: Test if next is a valid operator

        ast_t* rhs = parse_expression_primary(parser);
        if(!rhs) {
            parser_throw(parser, "Operator with missing second operand");
            return lhs;
        }

        int nextprec = parse_precedence(current_token(parser)->type);
        if(prec < nextprec) {
            rhs = parse_expression_last(parser, rhs, prec + 1);
            if(!rhs) {
                return 0;
            }
        }

        // Float optimization
        if(lhs->class == AST_FLOAT && rhs->class == AST_INT) {
            // convert right hand side to float too
            rhs->class = AST_FLOAT;
            rhs->f = (double)rhs->i;
        }
        else if(lhs->class == AST_INT && rhs->class == AST_FLOAT) {
            lhs->class = AST_FLOAT;
            lhs->f = (double)lhs->i;
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
 * parse_expression:
 * Parses one expression.
 *
 * EBNF:
 * Expression = Expression_primary { Operator, Expression_primary } .
 */
ast_t* parse_expression(parser_t* parser) {
    ast_t* lhs = parse_expression_primary(parser);
    if(!lhs) return lhs;

    // TODO: test if next is a valid operator
    if(!match_type(parser, TOKEN_NEWLINE)) {
        lhs = parse_expression_last(parser, lhs, 0);
    }

    return lhs;
}

/**
 * parse_datatype:
 * Reads a datatype.
 *
 * EBNF:
 * BaseType = "int" | "char" | "bool" | "float" | "generic" | "str" | "void" | TOKEN_WORD .
 * SimpleType = BaseType { "[]" } .
 * OptionType = "option" TOKEN_LESS ( SimpleType | OptionType ) TOKEN_GREATER .
 * Datatype = SimpleType | OptionType .
 */
datatype_t* parse_datatype(parser_t* parser) {
    token_t* typestr = accept_token_type(parser, TOKEN_WORD);
    if(!typestr) {
        parser_throw(parser, "Type must be an identifier, invalid");
        return context_null(parser->context);
    }

    datatype_t* t = context_get(parser->context, typestr->value);
    if(!t) {
        t = datatype_new(DATA_CLASS);
        t->id = djb2((unsigned char*)typestr->value);
        context_insert(parser->context, typestr->value, t);
    }

    if(datatype_match(t, context_get(parser->context, "option"))) {
        token_t* lt = accept_token_type(parser, TOKEN_LESS);
        if(!lt) {
            parser_throw(parser, "Expected opening bracket (<)");
            return context_null(parser->context);
        }

        datatype_t* subtype = parse_datatype(parser);
        token_t* gt = accept_token_type(parser, TOKEN_GREATER);
        if(!gt) {
            parser_throw(parser, "Expected closing bracket (>)");
            return context_null(parser->context);
        }

        datatype_t dt;
        dt.type = DATA_OPTION;
        dt.id = 0;
        dt.subtype = subtype;
        t = context_find_or_create(parser->context, &dt);
    }

    while(match_type(parser, TOKEN_LBRACKET)) {
        parser->cursor++;
        token_t* rbrac = accept_token_type(parser, TOKEN_RBRACKET);
        if(!rbrac) {
            parser_throw(parser, "Expected closing bracket");
            return context_null(parser->context);
        }

        if(t->type == DATA_VOID) {
            parser_throw(parser, "Invalid: array of type void");
            return context_null(parser->context);
        }

        datatype_t dt;
        dt.type = DATA_ARRAY;
        dt.id = 0;
        dt.subtype = t;
        t = context_find_or_create(parser->context, &dt);
    }

    return t;
}

/**
 * parse_formals:
 * The function parses the paramters of a function.
 * Every parameter is stored in a variable declaration AST
 * and returned in a list.
 *
 * EBNF:
 * Formals = "(" [ formal { "," formal } ] ")" .
 * Formal = [ "mut" ] TOKEN_WORD ":" Datatype .
 */
list_t* parse_formals(parser_t* parser) {
    list_t* formals = list_new();

    token_t* lparen = accept_token_type(parser, TOKEN_LPAREN);
    if(!lparen) {
        parser_throw(parser, "Missing left parenthesis");
        return formals;
    }

    while(!match_type(parser, TOKEN_RPAREN)) {
        // Mutable parameter
        bool mutable = false;
        if(match_type(parser, TOKEN_MUT)) {
            parser->cursor++;
            mutable = true;
        }

        if(!match_type(parser, TOKEN_WORD)) {
            parser_throw(parser, "Invalid argument list");
            return formals;
        }

        token_t* name = accept_token(parser);
        if(!match_type(parser, TOKEN_COLON)) {
            parser_throw(parser, "Type expected");
            return formals;
        }
        parser->cursor++; // ':'

        ast_t* param = ast_class_create(AST_DECLVAR, get_location(parser));
        param->vardecl.name = name->value;
        param->vardecl.type = parse_datatype(parser);
        param->vardecl.mutate = mutable;
        list_push(formals, param);

        if(!match_type(parser, TOKEN_COMMA) && !match_type(parser, TOKEN_RPAREN)) {
            parser_throw(parser, "Expected seperator");
            return formals;
        } else if(match_type(parser, TOKEN_COMMA)) {
            parser->cursor++;
        }
    }

    token_t* rparen = accept_token_type(parser, TOKEN_RPAREN);
    if(!rparen) {
        parser_throw(parser, "Missing right parenthesis");
    }

    return formals;
}

/**
 * parse_block:
 * Parses a block until a closing brace is reached.
 * Every statement is returned in a list.
 *
 * EBNF:
 * Block = "{" Newline { Statement } "}" .
 */
list_t* parse_block(parser_t* parser) {
    list_t* statements = list_new();
    if(!match_type(parser, TOKEN_LBRACE)) {
        parser_throw(parser, "Expected opening brace");
        return statements;
    }
    parser->cursor++;

    if(!match_type(parser, TOKEN_NEWLINE)) {
        parser_throw(parser, "Newline after block begin expected");
        return statements;
    }
    parser->cursor++;
    skip_newline(parser);

    while(!match_type(parser, TOKEN_RBRACE) && !parser_error(parser)) {
        if(parser_end(parser)) {
            parser_throw(parser, "Block not closed, reached end");
            return statements;
        }

        ast_t* stmt = parse_stmt(parser);
        if(!stmt) {
            return statements;
        }

        list_push(statements, stmt);
        skip_newline(parser);
    }

    if(!parser_error(parser)) {
        token_t* rbrace = accept_token_type(parser, TOKEN_RBRACE);
        if(!rbrace) {
            parser_throw(parser, "Missing closing brace");
        }
    }

    return statements;
}

// Newline testing, throws error if there is none
// Newline = '\r\n' | '\n';
void test_newline(parser_t* parser) {
    if(!parser->error) {
        if(!match_type(parser, TOKEN_NEWLINE)) {
            parser_throw(parser, "Invalid statement (Newline missing?)");
        } else {
            parser->cursor++;
        }
    }
}

/**
 * parse_stmt:
 * Parses a statement.
 * Every statement is followed / ended by a newline.
 *
 * EBNF:
 * Newline = TOKEN_NEWLINE .
 * Statement = Import | Variable | Function | If | While | Class | Return | Annotation | Expression .
 */
ast_t* parse_stmt(parser_t* parser) {
    location_t pos = get_location(parser);

    // Look-up-table
    static const struct {
        token_type_t ty;
        ast_t* (*fn)(parser_t*, location_t);
    } parsers[] = {
        {TOKEN_USING, parse_import_declaration},
        {TOKEN_LET, parse_var_declaration},
        {TOKEN_FUNC, parse_fn_declaration},
        {TOKEN_IF, parse_if_declaration},
        {TOKEN_WHILE, parse_while_declaration},
        {TOKEN_TYPE, parse_class_declaration},
        {TOKEN_RETURN, parse_return_declaration}
    };

    // Test for a statement
    // If there is a statement, test for a newline
    for(size_t i = 0; i < sizeof(parsers)/sizeof(parsers[0]); i++) {
        if(match_type(parser, parsers[i].ty)) {
            ast_t* node = parsers[i].fn(parser, pos);
            test_newline(parser);
            return node;
        }
    }

    // Annotation testing
    if(match_type(parser, TOKEN_AT)) {
        ast_t* node = parse_annotation_declaration(parser, pos);
        test_newline(parser);
        return node;
    }

    // Error testing
    if(match_type(parser, TOKEN_ELSE)) {
        parser_throw(parser, "If-clause error: No beginning if-statement / Else-if out of if-chain");
        return 0;
    }

    // Otherwise just do expression testing
    ast_t* node = parse_expression(parser);
    if(node) {
        return node;
    }

    token_t* token = current_token(parser);
    parser_throw(parser, "Could not interpret token '%s'", token->value);
    return 0;
}

///////-----------------
// Main parsing function
//////------------------

ast_t* parser_run(parser_t* parser, char* content) {
    parser->buffer = lexer_scan(parser->name, content, &parser->num_tokens);
    if(!parser->buffer) return 0;

    // Use this for lexical analysis, debug if tokens are read wrong
    // lexer_print_tokens(parser->buffer, parser->num_tokens);

    // Create toplevel program scope
    ast_t* ast = ast_class_create(AST_TOPLEVEL, get_location(parser));
    ast->toplevel = list_new();
    parser->top = ast;

    // Parse each statement and abort if an error occurs
    skip_newline(parser);
    while(!parser_end(parser)) {
        ast_t* node = parse_stmt(parser);
        if(!node || parser_error(parser)) {
            ast_free(node);
            ast_free(ast);
            parser->top = 0;
            return 0;
        }

        // add node to program
        list_push(ast->toplevel, node);
        skip_newline(parser);
    }

    return ast;
}

///////-----------------
// Parsing subroutines
//////------------------

/**
 * parse_import:
 * Parses an import statement and handles internal libraries.
 *
 * EBNF:
 * Import = "using" (TOKEN_WORD | TOKEN_STRING) Newline .
 */
ast_t* parse_import_declaration(parser_t* parser, location_t loc) {
    ast_t* node = ast_class_create(AST_IMPORT, loc);
    parser->cursor++; // import keyword

    // Built-in library handling
    if(match_type(parser, TOKEN_WORD)) {
        token_t* val = accept_token(parser);
        node->import = val->value;

    // External file handling
    } else if(match_type(parser, TOKEN_STRING)) {
        token_t* val = accept_token(parser);
        node->import = val->value;
    } else {
        parser_throw(parser, "Malformed import statement");
    }

    return node;
}

/**
 * parse_var_declaration:
 * Parses a variable declaration, returns AST of class 'AST_DECLVAR'.
 *
 * EBNF:
 * Variable = "let" [ "mut" ] TOKEN_WORD "=" Expression Newline .
 */
ast_t* parse_var_declaration(parser_t* parser, location_t loc) {
    ast_t* node = ast_class_create(AST_DECLVAR, loc);
    parser->cursor++; // let keyword

    // Test for the 'mutable'-keyword
    bool mutate = false;
    if(match_type(parser, TOKEN_MUT)) {
        parser->cursor++;
        mutate = true;
    }

    // ident, '='
    token_t* ident = accept_token_type(parser, TOKEN_WORD);
    token_t* eq = accept_token_type(parser, TOKEN_EQUAL);

    // If WORD followed by EQUAL
    if(ident && eq) {
        node->vardecl.name = ident->value;
        node->vardecl.mutate = mutate;
        node->vardecl.initializer = parse_expression(parser);
        node->vardecl.type = context_null(parser->context);

        if(!node->vardecl.initializer) {
            parser_throw(parser, "Invalid or missing variable initializer");
        }
    } else {
        parser_throw(parser, "Malformed variable declaration");
        node->class = AST_NULL;
    }

    return node;
}

/**
 * parse_fn_declaration:
 * Parses a function declaration.
 * If there is no arrow with a datatype, the function is of type void.
 *
 * EBNF:
 * Function = "func" TOKEN_WORD Formals ( ( "->" Datatype Block ) | Block ) Newline .
 */
ast_t* parse_fn_declaration(parser_t* parser, location_t loc) {
    ast_t* node = ast_class_create(AST_DECLFUNC, loc);
    parser->cursor++; // function keyword

    // Identifier
    token_t* ident = accept_token_type(parser, TOKEN_WORD);
    if(ident) {
        node->funcdecl.name = ident->value;
        node->funcdecl.impl.formals = parse_formals(parser);
        node->funcdecl.rettype = context_null(parser->context);
        node->funcdecl.external = false;

        // Make arrow optional
        if(match_type(parser, TOKEN_LBRACE)) {
            // function is of type void
            // there is no datatype

            node->funcdecl.rettype = context_void(parser->context);
        } else if(match_type(parser, TOKEN_ARROW)) {
            parser->cursor++;
            node->funcdecl.rettype = parse_datatype(parser);
        } else {
            parser_throw(parser, "Return type expected");
            return node;
        }

        node->funcdecl.impl.body = parse_block(parser);
    } else {
        parser_throw(parser, "Malformed function declaration");
    }

    return node;
}

/**
 * parse_if_declaration:
 * Parses an if-statment-chain, consisting of:
 * an if-statement, optionally multiple else-if-statements
 * and optionally one else statement.
 *
 * EBNF:
 * If = "if" Expression Block { ElseIf } [ Else ] .
 * ElseIf = "else" "if" Expression Block .
 * Else = "else" Block .
 *
 * Example:
 * if cond1 {
 *     do1()
 * } else if cond2 {
 *     do2()
 * } else {
 *     do3()
 * }
 */
ast_t* parse_if_declaration(parser_t* parser, location_t loc) {
    ast_t* node = ast_class_create(AST_IF, loc);
    node->ifstmt = list_new();

    // if (if / else if) do
    while(match_type(parser, TOKEN_IF)
        || (match_type(parser, TOKEN_ELSE)
        && match_next(parser, TOKEN_IF))) {
        // create a subclause and skip tokens
        ast_t* clause = ast_class_create(AST_IFCLAUSE, get_location(parser));
        if(match_type(parser, TOKEN_IF)) {
            accept_token_type(parser, TOKEN_IF);
        } else {
            accept_token_type(parser, TOKEN_ELSE);
            accept_token_type(parser, TOKEN_IF);
        }

        clause->ifclause.cond = parse_expression(parser);
        clause->ifclause.body = parse_block(parser);
        list_push(node->ifstmt, clause);
    }

    if(match_type(parser, TOKEN_ELSE)) {
        ast_t* clause = ast_class_create(AST_IFCLAUSE, get_location(parser));
        token_t* elsetok = accept_token_type(parser, TOKEN_ELSE);
        if(elsetok) {
            clause->ifclause.cond = 0;
            clause->ifclause.body = parse_block(parser);
        } else {
            parser_throw(parser, "Else statement without else keyword");
            free(clause);
            return node;
        }

        list_push(node->ifstmt, clause);
    }

    return node;
}

/**
 * parse_while_declaration:
 * Builds an ast for a while loop.
 *
 * EBNF:
 * While = "while" Expression Block Newline .
 */
ast_t* parse_while_declaration(parser_t* parser, location_t loc) {
    // while expr { \n
    ast_t* node = ast_class_create(AST_WHILE, loc);
    parser->cursor++; // while keyword

    node->whilestmt.cond = parse_expression(parser);
    node->whilestmt.body = parse_block(parser);
    return node;
}

/**
 * parse_class_declaration:
 * Parses a class declaration.
 *
 * EBNF:
 * Class = "type" TOKEN_WORD Formals Block .
 */
ast_t* parse_class_declaration(parser_t* parser, location_t loc) {
    // class expr(constructor) { \n
    ast_t* node = ast_class_create(AST_CLASS, loc);
    parser->cursor++;

    // Identifier
    token_t* ident = accept_token_type(parser, TOKEN_WORD);
    if(ident) {
        node->classstmt.name = ident->value;
        node->classstmt.formals = parse_formals(parser);
        node->classstmt.body = parse_block(parser);
        node->classstmt.fields = hashmap_new();
    } else {
        parser_throw(parser, "Malformed class declaration");
    }

    return node;
}

/**
 * parse_return_declaration:
 * Parses a return statement.
 *
 * EBNF:
 * Return = "return" [ Expression ] Newline .
 */
ast_t* parse_return_declaration(parser_t* parser, location_t loc) {
    ast_t* node = ast_class_create(AST_RETURN, loc);
    parser->cursor++;

    if(match_type(parser, TOKEN_NEWLINE)) {
        node->returnstmt = 0;
    } else {
        node->returnstmt = parse_expression(parser);
    }

    return node;
}

/**
 * parse_annotation_declaration:
 * Parses an annotation.
 *
 * EBNF:
 * Annotation = "@" ( "Getter" | "Setter" | "Unused" ) Newline .
 */
ast_t* parse_annotation_declaration(parser_t* parser, location_t loc) {
    ast_t* node = ast_class_create(AST_ANNOTATION, loc);
    token_t* keyword = accept_token_type(parser, TOKEN_AT);
    token_t* content = accept_token_type(parser, TOKEN_WORD);

    if(keyword && content) {
        char* str = content->value;
        if(!strcmp(str, "Getter")) {
            node->annotation = ANN_GETTER;
        } else if(!strcmp(str, "Setter")) {
            node->annotation = ANN_SETTER;
        } else if(!strcmp(str, "Unused")) {
            node->annotation = ANN_UNUSED;
        } else {
            parser_throw(parser, "Unknown annotation type");
        }
    } else {
        parser_throw(parser, "Malformed annotation");
    }

    return node;
}
