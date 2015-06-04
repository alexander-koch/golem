#include "compiler.h"

void compiler_eval(list_t* buffer, ast_t* node);

// Evaluates a block and returns the last value
void eval_block(list_t* buffer, list_t* block)
{
	list_iterator_t* iter = list_iterator_create(block);
	while(!list_iterator_end(iter))
	{
		compiler_eval(buffer, list_iterator_next(iter));
	}
	list_iterator_free(iter);
}

void eval_declfunc(list_t* buffer, ast_t* node)
{
	// Example:
	// PUSH "ARG2"
	// PUSH "ARG1"
	// PUSH "ARG0"
	// PUSH 3
	// STORE "function name"

	/*I64 args = list_size(node->funcdecl.impl.formals);
	list_iterator_t* iter = list_iterator_create(node->funcdecl.impl.formals);
	while(!list_iterator_end(iter))
	{
		char* str = (char*)list_iterator_next(iter);
		emit_string(buffer, str);
	}
	list_iterator_free(iter);

	emit_i64(buffer, args);
	emit_store(buffer, false, node->funcdecl.name);*/
}

void eval_declvar(list_t* buffer, ast_t* node)
{
	// Example:
	// PUSH 5 			-> push 5 onto stack
	// PUSH 4 			-> push 4 onto stack
	// ADD				-> pop 2, add
	// STORE "variable"

	compiler_eval(buffer, node->vardecl.initializer);
	emit_store(buffer, node->vardecl.mutate, node->vardecl.name);
}

void eval_number(list_t* buffer, ast_t* node)
{
	if(node->class == AST_FLOAT)
	{
		emit_f64(buffer, node->f);
	}
	else
	{
		emit_i64(buffer, node->i);
	}
}

void eval_binary(list_t* buffer, ast_t* node)
{
	// Emit node op-codes
	compiler_eval(buffer, node->binary.left);
	compiler_eval(buffer, node->binary.right);

	// Emit operator
	token_type_t op = node->binary.op;
	emit_tok2op(buffer, op);
}

void eval_array(list_t* buffer, ast_t* node)
{
	// Example:
	// PUSH 5
	// PUSH 6
	// PUSH 67
	// ARRAY 3

	list_iterator_t* iter = list_iterator_create(node->array);
	while(!list_iterator_end(iter))
	{
		compiler_eval(buffer, list_iterator_next(iter));
	}
	list_iterator_free(iter);
	emit_array(buffer, list_size(node->array));
}

void eval_ident(list_t* buffer, ast_t* node)
{
	emit_load(buffer, node->ident);
}

void eval_call(list_t* buffer, ast_t* node)
{
	// PUSH 3
	// PUSH 5
	// PUSH HELLO
	// CALL "println" 3

	I64 args = list_size(node->call.args);
	list_iterator_t* iter = list_iterator_create(node->call.args);
	while(!list_iterator_end(iter))
	{
		ast_t* node = list_iterator_next(iter);
		compiler_eval(buffer, node);
	}
	list_iterator_free(iter);

	emit_call(buffer, node->call.callee->ident, args);
}

void eval_string(list_t* buffer, ast_t* node)
{
	emit_string(buffer, node->string);
}

void compiler_eval(list_t* buffer, ast_t* node)
{
	switch(node->class)
	{
		case AST_TOPLEVEL:
			{ eval_block(buffer, node->toplevel); break; }
		case AST_DECLVAR:
			{ eval_declvar(buffer, node); break; }
	/*	case AST_DECLFUNC:
			{ eval_declfunc(buffer, node); break; }*/
		case AST_FLOAT:
			{ eval_number(buffer, node); break; }
		case AST_INT:
			{ eval_number(buffer, node); break; }
		case AST_STRING:
			{ eval_string(buffer, node); break; }
		case AST_BINARY:
			{ eval_binary(buffer, node); break; }
		case AST_ARRAY:
			{ eval_array(buffer, node); break; }
		case AST_IDENT:
			{ eval_ident(buffer, node); break; }
		case AST_CALL:
			{ eval_call(buffer, node); break; }
		default: break;
	}
}

/**
 *	'Dumps' the node to the console.
 *	Basically a printing function for abstract syntax trees
 */
void compiler_dump(ast_t* node, int level)
{
	for(int i = 0; i < level; i++)
	{
		console("#");
	}

	switch(node->class)
	{
		case AST_TOPLEVEL:
		{
			list_iterator_t* iter = list_iterator_create(node->toplevel);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				compiler_dump(next, level+1);
				fprintf(stdout, "\n");
			}
			list_iterator_free(iter);
			break;
		}
		case AST_DECLVAR:
		{
			fprintf(stdout, ":decl %s<", node->vardecl.name);
			compiler_dump(node->vardecl.initializer, 0);
			fprintf(stdout, ">");
			break;
		}
		case AST_DECLFUNC:
		{
			fprintf(stdout, ":func<%s>\n", node->funcdecl.name);
			list_iterator_t* iter = list_iterator_create(node->funcdecl.impl.body);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				compiler_dump(next, level+1);
				fprintf(stdout, "\n");
			}
			list_iterator_free(iter);
			break;
		}
		case AST_IDENT:
		{
			fprintf(stdout, ":ident = %s", node->ident);
			break;
		}
		case AST_FLOAT:
		{
			fprintf(stdout, ":num = %f", node->f);
			break;
		}
		case AST_INT:
		{
			fprintf(stdout, ":num = %li", (long int)node->i);
			break;
		}
		case AST_STRING:
		{
			fprintf(stdout, ":str = '%s'", node->string);
			break;
		}
		case AST_BOOL:
		{
			fprintf(stdout, ":bool = '%s'", node->b == true ? "true" : "false");
			break;
		}
		case AST_BINARY:
		{
			fprintf(stdout, ":bin<");
			compiler_dump(node->binary.left, 0);
			compiler_dump(node->binary.right, 0);
			fprintf(stdout, ":op = %d>", node->binary.op);
			break;
		}
		case AST_UNARY:
		{
			fprintf(stdout, ":unary<");
			compiler_dump(node->unary.expr, 0);
			fprintf(stdout, ">");
			break;
		}
		case AST_SUBSCRIPT:
		{
			fprintf(stdout, ":subscript<(key)");
			compiler_dump(node->subscript.key, 0);
			fprintf(stdout, "; (expr)");
			compiler_dump(node->subscript.expr, 0);
			fprintf(stdout, ">");
			break;
		}
		case AST_IF:
		{
			fprintf(stdout, ":if<");
			compiler_dump(node->ifstmt.cond, 0);
			fprintf(stdout, ">\n");

			list_iterator_t* iter = list_iterator_create(node->ifstmt.body);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				compiler_dump(next, level+1);
				fprintf(stdout, "\n");
			}
			list_iterator_free(iter);

			break;
		}
		case AST_ARRAY:
		{
			fprintf(stdout, ":array<");

			list_iterator_t* iter = list_iterator_create(node->array);
			while(!list_iterator_end(iter))
			{
				compiler_dump(list_iterator_next(iter), 0);
			}
			list_iterator_free(iter);
			fprintf(stdout, ">");
			break;
		}
		case AST_CALL:
		{
			fprintf(stdout, ":call<");
			compiler_dump(node->call.callee, 0);
			fprintf(stdout, ">");
			break;
		}
		default: break;
	}
}

/**
 *	Creates an instruction set based on source code @source
 */
list_t* compile_buffer(compiler_t* compiler, const char* source)
{
	list_t* buffer = list_new();

	parser_init(&compiler->parser);
	ast_t* root = parser_run(&compiler->parser, source);
	if(root)
	{
		compiler_dump(root, 0);
		//compiler_eval(buffer, root);
		ast_free(root);
	}

	parser_free(&compiler->parser);
	return buffer;
}

/**
 *	Reads a file and converts it into a buffer
 */
list_t* compile_file(compiler_t* compiler, const char* filename)
{
	FILE* file = fopen(filename, "rb");
	if(!file) return 0;
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);
	char* source = (char*)malloc(size+1);
	fread(source, sizeof(char), size, file);
 	source[size] = '\0';
	fclose(file);

	// run vm with buffer
	list_t* buffer = compile_buffer(compiler, source);
	free(source);
	return buffer;
}

void buffer_free(list_t* buffer)
{
	list_iterator_t* iter = list_iterator_create(buffer);
	while(!list_iterator_end(iter))
	{
		instruction_t* instr = (instruction_t*)list_iterator_next(iter);
		if(instr->v1) value_free(instr->v1);
		if(instr->v2) value_free(instr->v2);
		free(instr);
	}
	list_iterator_free(iter);
	list_free(buffer);
}
