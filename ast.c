#include "ast.h"

ast_t* ast_class_create(ast_class_t class, location_t location)
{
	ast_t* ast = malloc(sizeof(*ast));
	if(!ast) return ast;

	ast->class = class;
	ast->location = location;
    return ast;
}

const char* ast_classname(ast_class_t class)
{
	switch(class)
	{
		case AST_IDENT: return "identifier";
		case AST_FLOAT: return "float";
		case AST_INT: return "integer";
		case AST_STRING: return "string";
		case AST_ARRAY: return "array";
		case AST_BINARY: return "binary";
		case AST_DECLVAR: return "variable declaration";
		case AST_TOPLEVEL: return "toplevel";
		case AST_CALL: return "call";
		default: return "null";
	}
}

void ast_free(ast_t* ast)
{
	if(!ast) return;

	list_iterator_t* iter = 0;
	switch(ast->class)
	{
		case AST_TOPLEVEL:
		{
			iter = list_iterator_create(&ast->toplevel);
			while(!list_iterator_end(iter))
			{
				ast_free(list_iterator_next(iter));
			}
			list_iterator_free(iter);
			list_free(&ast->toplevel);
			break;
		}
		case AST_DECLVAR:
		{
			//free(ast->vardecl.name);
			ast_free(ast->vardecl.initializer);
			break;
		}
		case AST_BINARY:
		{
			ast_free(ast->binary.left);
			ast_free(ast->binary.right);
			break;
		}
		case AST_CALL:
		{
			ast_free(ast->call.callee);

			iter = list_iterator_create(&ast->call.args);
			while(!list_iterator_end(iter))
			{
				ast_free(list_iterator_next(iter));
			}
			list_iterator_free(iter);
			list_free(&ast->call.args);
		}
		default: break;
	}
	free(ast);
}
