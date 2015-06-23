#include "compiler.h"

void compiler_eval(compiler_t* compiler, ast_t* node);

void eval_block(compiler_t* compiler, list_t* block)
{
	list_iterator_t* iter = list_iterator_create(block);
	while(!list_iterator_end(iter))
	{
		compiler_eval(compiler, list_iterator_next(iter));
	}
	list_iterator_free(iter);
}

void eval_declfunc(compiler_t* compiler, ast_t* node)
{
	emit_begin_func(compiler->buffer, node->funcdecl.name, list_size(node->funcdecl.impl.formals));

	list_iterator_t* iter = list_iterator_create(node->funcdecl.impl.formals);
	while(!list_iterator_end(iter))
	{
		char* token = list_iterator_next(iter);
		emit_string(compiler->buffer, token);
	}
	list_iterator_free(iter);

	eval_block(compiler, node->funcdecl.impl.body);
	emit_scope_end(compiler->buffer);
}

void eval_declvar(compiler_t* compiler, ast_t* node)
{
	compiler_eval(compiler, node->vardecl.initializer);
	emit_store_field(compiler->buffer, node->vardecl.name, node->vardecl.mutate);
}

void eval_number(compiler_t* compiler, ast_t* node)
{
	if(node->class == AST_FLOAT)
	{
		emit_f64(compiler->buffer, node->f);
	}
	else
	{
		emit_i64(compiler->buffer, node->i);
	}
}

void eval_binary(compiler_t* compiler, ast_t* node)
{
	token_type_t op = node->binary.op;
	if(op == TOKEN_ASSIGN)
	{
		ast_t* lhs = node->binary.left;
		if(lhs->class != AST_IDENT)
		{
			// TODO: throw error / eval subscript
			return;
		}

		compiler_eval(compiler, node->binary.right);
		emit_store_field(compiler->buffer,lhs->ident, false);
	}
	else
	{
		// Emit node op-codes
		compiler_eval(compiler, node->binary.left);
		compiler_eval(compiler, node->binary.right);

		// Emit operator
		emit_tok2op(compiler->buffer,op);
	}
}

void eval_ident(compiler_t* compiler, ast_t* node)
{
	emit_get_field(compiler->buffer, node->ident);
}

void eval_call(compiler_t* compiler, ast_t* node)
{
	I64 args = list_size(node->call.args);
	eval_block(compiler, node->call.args);

	ast_t* call = node->call.callee;
	if(call->class == AST_IDENT)
	{
		emit_string(compiler->buffer,call->ident);
	}
	else
	{
		// TODO: FIX!!
		// Subscript?
	}
	emit_invoke(compiler->buffer,args);
}

void eval_string(compiler_t* compiler, ast_t* node)
{
	emit_string(compiler->buffer, node->string);
}

void eval_if(compiler_t* compiler, ast_t* node)
{
	// TODO: implement
}

void compiler_eval(compiler_t* compiler, ast_t* node)
{
	switch(node->class)
	{
		case AST_TOPLEVEL:
		{
			eval_block(compiler, node->toplevel);
			break;
		}
		case AST_DECLVAR:
		{
			eval_declvar(compiler, node);
			break;
		}
		case AST_DECLFUNC:
		{
			eval_declfunc(compiler, node);
			break;
		}
		case AST_FLOAT:
		{
			eval_number(compiler, node);
			break;
		}
		case AST_INT:
		{
			eval_number(compiler, node);
			break;
		}
		case AST_STRING:
		{
			eval_string(compiler, node);
			break;
		}
		case AST_BINARY:
		{
			eval_binary(compiler, node);
			break;
		}
		case AST_IDENT:
		{
			eval_ident(compiler, node);
			break;
		}
		case AST_CALL:
		{
			eval_call(compiler, node);
			break;
		}
		case AST_IF:
		{
			eval_if(compiler, node);
			break;
		}
		default: break;
	}
}

/**
 *	'Dumps' the node to the console.
 *	Basically a printing function for abstract syntax trees
 */
void compiler_dump(ast_t* node, int level)
{
	if(!node) return;

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
			fprintf(stdout, ":op = %s>", tok2str(node->binary.op));
			break;
		}
		case AST_UNARY:
		{
			fprintf(stdout, ":unary<%s, ", tok2str(node->unary.op));
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
			fprintf(stdout, ":if<>\n");
			list_iterator_t* iter = list_iterator_create(node->ifstmt);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				compiler_dump(next, level+1);
				fprintf(stdout, "\n");
			}
			list_iterator_free(iter);
			break;
		}
		case AST_IFCLAUSE:
		{
			if(node->ifclause.cond != 0)
			{
				fprintf(stdout, ":ifclause<");
				compiler_dump(node->ifclause.cond, 0);
				fprintf(stdout, ">");
			}
			else
			{
				fprintf(stdout, ":else<>");
			}

			if(list_size(node->ifclause.body) > 0)
			{
				fprintf(stdout, "\n");
				list_iterator_t* iter = list_iterator_create(node->ifclause.body);
				while(!list_iterator_end(iter))
				{
					ast_t* next = list_iterator_next(iter);
					compiler_dump(next, level+1);
					fprintf(stdout, "\n");
				}
				list_iterator_free(iter);
			}
			break;
		}
		case AST_WHILE:
		{
			fprintf(stdout, ":while<");
			compiler_dump(node->whilestmt.cond, 0);
			fprintf(stdout, ">\n");

			list_iterator_t* iter = list_iterator_create(node->whilestmt.body);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				compiler_dump(next, level+1);
				fprintf(stdout, "\n");
			}
			list_iterator_free(iter);
			break;
		}
		case AST_INCLUDE:
		{
			fprintf(stdout, ":use<");
			compiler_dump(node->include, 0);
			fprintf(stdout, ">");
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
		case AST_CLASS:
		{
			fprintf(stdout, ":class<%s>\n", node->classstmt.name);
			list_iterator_t* iter = list_iterator_create(node->classstmt.body);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				compiler_dump(next, level+1);
				fprintf(stdout, "\n");
			}
			list_iterator_free(iter);
			break;
		}
		case AST_RETURN:
		{
			fprintf(stdout, ":return<");
			compiler_dump(node->returnstmt, 0);
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

void llvm_compile(compiler_t* cmpl, ast_t* root)
{
#ifdef __USE_LLVM__
	llvm_context_t* ctx = llvm_context_create(cmpl->filename);
	llvm_traverse(ctx, root);

	if(cmpl->filename)
	{
		char* ext = strdup(".bc");
		char* out = concat(cmpl->filename, ext);

		fprintf(stdout, "Writing to file...\n");
		// Write out bitcode to file
	    if (LLVMWriteBitcodeToFile(ctx->module, out) != 0) {
	        fprintf(stderr, "Error writing bitcode to file, skipping\n");
	    }

		free(ext);
		free(cmpl->filename);
	}

	LLVMDumpModule(ctx->module);
	llvm_context_free(ctx);
#endif
}

/**
 *	Creates an instruction set based on source code @source
 */
list_t* compile_buffer(compiler_t* compiler, const char* source)
{
	compiler_clear(compiler);
	compiler->buffer = list_new();

	parser_init(&compiler->parser);
	ast_t* root = parser_run(&compiler->parser, source);
	if(root)
	{
		compiler_dump(root, 0);
		// compiler_eval(compiler, root);
		llvm_compile(compiler, root);

		ast_free(root);
	}

	parser_free(&compiler->parser);
	return compiler->buffer;
}

/**
 *	Reads a file and converts it into a compiler
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

	// run vm with compiler
	list_t* buffer = compile_buffer(compiler, source);
	free(source);

	compiler->filename = strdup(filename);
	return buffer;
}

void compiler_clear(compiler_t* compiler)
{
	if(compiler->buffer)
	{
		list_iterator_t* iter = list_iterator_create(compiler->buffer);
		while(!list_iterator_end(iter))
		{
			instruction_t* instr = (instruction_t*)list_iterator_next(iter);
			if(instr->v1) value_free(instr->v1);
			if(instr->v2) value_free(instr->v2);
			free(instr);
		}
		list_iterator_free(iter);
		list_free(compiler->buffer);
		compiler->buffer = 0;
	}
}
