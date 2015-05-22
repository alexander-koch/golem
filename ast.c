#include "ast.h"

ast_t* ast_class_create(ast_class_t class, location_t location)
{
	ast_t* ast = malloc(sizeof(*ast));
	if(!ast) return ast;

	ast->class = class;
	ast->location = location;
    return ast;
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
		default: break;
	}
	free(ast);
}
