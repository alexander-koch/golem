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
			ast_t* lhs = node->binary.left;
			ast_t* rhs = node->binary.right;

			if(lhs->class != rhs->class)
			{
				opt_throw(opt, "Cannot compare objects of different classes");
				break;
			}

			// Do string testing
			// Form:
			// (String + String) -> String
			// (String + Int) = Invalid
			// (String + Float) = Invalid
			// [...]

			if((lhs->class == AST_STRING && rhs->class != AST_STRING) ||
				(lhs->class != AST_STRING && rhs->class == AST_STRING))
			{
				opt_throw(opt, "Invalid statement, objects of type string can only be compared with a another string");
				break;
			}

			// Do array testing
			// (Array + Array) = Array

			if((lhs->class == AST_ARRAY && rhs->class != AST_ARRAY) ||
				(lhs->class != AST_ARRAY && lhs->class == AST_ARRAY))
			{
				opt_throw(opt, "Invalid statement, objects of type array can only be compared with another array");
				break;
			}

			if(lhs->class == AST_FLOAT && rhs->class == AST_FLOAT)
	        {
	            switch(node->binary.op)
	            {
	                case TOKEN_ADD:
	                {
	                    lhs->f = lhs->f + rhs->f;
	                    break;
	                }
	                case TOKEN_SUB:
	                {
	                    lhs->f = lhs->f - rhs->f;
	                    break;
	                }
	                case TOKEN_MUL:
	                {
	                    lhs->f = lhs->f * rhs->f;
	                    break;
	                }
	                case TOKEN_DIV:
	                {
	                    lhs->f = lhs->f / rhs->f;
	                    break;
	                }
	                case TOKEN_EQUAL:
	                {
	                    lhs->class = AST_BOOL;
	                    lhs->b = (lhs->f == rhs->f);
	                    break;
	                }
	                case TOKEN_NEQUAL:
	                {
	                    lhs->class = AST_BOOL;
	                    lhs->b = (lhs->f != rhs->f);
	                    break;
	                }
	                case TOKEN_LEQUAL:
	                {
	                    lhs->class = AST_BOOL;
	                    lhs->b = (lhs->f <= lhs->f);
	                    break;
	                }
	                case TOKEN_GEQUAL:
	                {
	                    lhs->class = AST_BOOL;
	                    lhs->b = (lhs->f >= lhs->f);
	                    break;
	                }
	                case TOKEN_LESS:
	                {
	                    lhs->class = AST_BOOL;
	                    lhs->b = (lhs->f < lhs->f);
	                    break;
	                }
	                case TOKEN_GREATER:
	                {
	                    lhs->class = AST_BOOL;
	                    lhs->b = (lhs->f > lhs->f);
	                    break;
	                }
	                default:
	                {
	                    opt_throw(opt, "Invalid operator. Operator might not be available for floats");
	                    return;
	                }
	            }
	        }

			if(lhs->class == AST_STRING && rhs->class == AST_STRING)
			{
				switch(node->binary.op)
				{
					case TOKEN_ADD:
					{
						char* tmp = concat(lhs->string, rhs->string);
						free(lhs->string);
						lhs->string = tmp;
						break;
					}
					case TOKEN_EQUAL:
	                {
	                    lhs->class = AST_BOOL;
	                    lhs->b = strcmp(lhs->string, rhs->string) == 0;
						free(lhs->string);
	                    break;
	                }
					default:
					{
						opt_throw(opt, "Invalid operator. Operator might not be available for strings");
						return;
					}
				}
			}


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
			datatype_t rettp = node->funcdecl.rettype;
			ast_t* ret = 0;

			// Iterate through body
			list_iterator_t* iter = list_iterator_create(node->funcdecl.impl.body);
			while(!list_iterator_end(iter))
			{
				ast_t* sub = list_iterator_next(iter);

				// If return reached before end
				if(sub->class == AST_RETURN && !list_iterator_end(iter))
				{
					opt->node = sub;
					opt_throw(opt, "Return statement declared before end was reached");
					break;
				}

				if(sub->class == AST_RETURN) ret = sub;
			}
			list_iterator_free(iter);

			if(ret && rettp == DATA_VOID)
			{
				if(ret->returnstmt)
				{
					opt->node = ret;
					opt_throw(opt, "Void functions do not return a value");
					break;
				}
			}

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
