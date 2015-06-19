#include "optimizer.h"

typedef struct
{
	ast_t* node;
	bool error;
} optimizer_t;

void opt_throw(optimizer_t* opt, const char* format, ...)
{
	opt->error = true;
    location_t loc = opt->node->location;
    fprintf(stdout, "[line %d, column %d] (Semantic): ", loc.line, loc.column);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
    fprintf(stdout, "\n");
}

void optimize_node(optimizer_t* opt, ast_t* node)
{
	opt->node = node;
	switch(node->class)
	{
		case AST_TOPLEVEL:
		{
			list_iterator_t* iter = list_iterator_create(node->toplevel);
			while(!list_iterator_end(iter))
			{
				optimize_node(opt, list_iterator_next(iter));
			}
			list_iterator_free(iter);
			break;
		}
		case AST_BINARY:
		{
			optimize_node(opt, node->binary.left);
			optimize_node(opt, node->binary.right);
			break;
		}
		case AST_CLASS:
		{
			list_iterator_t* iter = list_iterator_create(node->classstmt.body);
			while(!list_iterator_end(iter))
			{
				ast_t* sub = list_iterator_next(iter);
				if(sub->class == AST_DECLFUNC || sub->class == AST_DECLVAR)
				{
					optimize_node(opt, sub);
				}
				else
				{
					opt->node = sub;
					opt_throw(opt, "Statements are not allowed a direct field of a class (node: %s)", ast_classname(sub->class));
				}
			}
			list_iterator_free(iter);
			break;
		}
		case AST_DECLFUNC:
		{
			// Body analysing
			list_iterator_t* iter = list_iterator_create(node->funcdecl.impl.body);
			while(!list_iterator_end(iter))
			{
				ast_t* sub = list_iterator_next(iter);
				if(sub->class == AST_RETURN && !list_iterator_end(iter))
				{
					opt->node = sub;
					opt_throw(opt, "Return statement declared before end was reached");
					break;
				}
			}
			list_iterator_free(iter);

			// Formals analysing
			iter = list_iterator_create(node->funcdecl.impl.formals);
			while(!list_iterator_end(iter))
			{
				param_t* sub = list_iterator_next(iter);
				if(sub->type == DATA_VOID || ((sub->type & DATA_ARRAY) == DATA_ARRAY && (sub->type & DATA_VOID) == DATA_VOID))
				{
					opt_throw(opt, "Parameters may not be declared as void");
					break;
				}
			}
			list_iterator_free(iter);

			break;
		}
		default: break;
	}
}

bool optimize_tree(ast_t* node)
{
	optimizer_t opt;
	opt.node = node;
	opt.error = false;

	if(node->class == AST_TOPLEVEL)
	{
		optimize_node(&opt, node);
	}
	else
	{
		opt_throw(&opt, "Node must be a root node");
	}

	return !opt.error;
}
