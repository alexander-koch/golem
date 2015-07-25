#include "compiler.h"

datatype_t compiler_eval(compiler_t* compiler, ast_t* node);

void compiler_init(compiler_t* compiler, bool debugmode)
{
	compiler->buffer = 0;
	compiler->filename = 0;
	compiler->debug = debugmode;
	compiler->error = false;
}

void compiler_throw(compiler_t* compiler, ast_t* node, const char* format, ...)
{
	compiler->error = true;
    location_t loc = node->location;
    fprintf(stdout, "[line %d, column %d] (Semantic): ", loc.line, loc.column);
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
    fprintf(stdout, "\n");
}

datatype_t eval_block(compiler_t* compiler, list_t* block)
{
	datatype_t ret = DATA_NULL;

	list_iterator_t* iter = list_iterator_create(block);
	while(!list_iterator_end(iter))
	{
		ret = compiler_eval(compiler, list_iterator_next(iter));
	}
	list_iterator_free(iter);
	return ret;
}

datatype_t eval_declfunc(compiler_t* compiler, ast_t* node)
{
	// TESTIFICATE
	// list_iterator_t* iter = list_iterator_create(node->funcdecl.impl.formals);
	// while(!list_iterator_end(iter))
	// {
	// 	char* token = list_iterator_next(iter);
	// 	//emit_string(compiler->buffer, token);
	// }
	// list_iterator_free(iter);
	//
	// eval_block(compiler, node->funcdecl.impl.body);
	return DATA_NULL;
}

datatype_t eval_declvar(compiler_t* compiler, ast_t* node)
{
	// First eval initializer
	datatype_t vartype = compiler_eval(compiler, node->vardecl.initializer); // <-- get type

	// Store symbol
	symbol_t* symbol = malloc(sizeof(*symbol));
	symbol->node = node;
	symbol->address = compiler->address;
	symbol->node->vardecl.type = vartype;
	hashmap_set(compiler->symbols, node->vardecl.name, symbol);

	// Emit last bytecode
	emit_store(compiler->buffer, symbol->address);

	// Increase compiler address
	compiler->address++;

	// Debug variables
	//console("Created variable %s: %s with data type %d\n", node->vardecl.name, node->vardecl.mutate ? "mut" : "immut", vartype);

	return DATA_NULL;
}

datatype_t eval_number(compiler_t* compiler, ast_t* node)
{
	if(node->class == AST_FLOAT)
	{
		emit_float(compiler->buffer, node->f);
		return DATA_FLOAT;
	}

	emit_int(compiler->buffer, node->i);
	return DATA_INT;
}

datatype_t eval_binary(compiler_t* compiler, ast_t* node)
{
	ast_t* lhs = node->binary.left;
	ast_t* rhs = node->binary.right;
	token_type_t op = node->binary.op;

	// Testing and internal optimization

	// Do string testing
	// String x String -> String
	// String x Other = Invalid

	if((lhs->class == AST_STRING && rhs->class != AST_STRING) ||
		(lhs->class != AST_STRING && rhs->class == AST_STRING))
	{
		compiler_throw(compiler, node, "Invalid statement, objects of type string can only be compared with a another string");
		return DATA_NULL;
	}

	if(lhs->class == AST_FLOAT && rhs->class == AST_FLOAT)
	{
		switch(op)
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
				compiler_throw(compiler, node, "Invalid operator. Operator might not be available for floats");
				return DATA_NULL;
			}
		}
	}

	// How to eval properly?

	// Immutability check
	if(op == TOKEN_ASSIGN)
	{
		if(lhs->class == AST_IDENT)
		{
			void* val;
			if(hashmap_get(compiler->symbols, lhs->ident, &val) != HMAP_MISSING)
			{
				symbol_t* symbol = (symbol_t*)val;
				if(symbol->node->class == AST_DECLVAR)
				{
					if(!symbol->node->vardecl.mutate)
					{
						compiler_throw(compiler, node, "Invalid statement, trying to modifiy an immutable variable");
						return DATA_NULL;
					}

					compiler_eval(compiler, rhs); // gettype here
					emit_store(compiler->buffer, symbol->address);
				}
				else
				{
					// TODO: other types
				}
			}
			else
			{
				compiler_throw(compiler, node, "Warning: Implicit declaration of field '%s'", lhs->ident);
				return DATA_NULL;
			}
		}
		else
		{
			// We got a subscript, todo!!
		}
	}
	else
	{
		// Emit node op-codes
		datatype_t lhs_type = compiler_eval(compiler, lhs);
		datatype_t rhs_type = compiler_eval(compiler, rhs);

		if(lhs_type != rhs_type)
		{
			compiler_throw(compiler, node, "Cannot perform operation '%s' on the types '%s' and '%s'", tok2str(op), datatype2str(lhs_type), datatype2str(rhs_type));
			return DATA_NULL;
		}

		// Emit operator
		emit_tok2op(compiler->buffer, op, lhs_type);
		return lhs_type;
	}
	return DATA_NULL;
}

datatype_t eval_ident(compiler_t* compiler, ast_t* node)
{
	void* val = 0;
	if(hashmap_get(compiler->symbols, node->ident, &val) != HMAP_MISSING)
	{
		symbol_t* ptr = (symbol_t*)val;
		emit_load(compiler->buffer, ptr->address);

		if(ptr->node->class == AST_DECLVAR)
		{
			return ptr->node->vardecl.type;
		}
		else if(node->class == AST_CLASS)
		{
			// TODO:
		}
		else if(node->class == AST_SUBSCRIPT)
		{
			// TODO:
		}

		return DATA_NULL;
	}

	compiler_throw(compiler, node, "Warning: Implicit declaration of field '%s'", node->ident);
	return DATA_NULL;
}

datatype_t eval_call(compiler_t* compiler, ast_t* node)
{
	size_t args = list_size(node->call.args);
	eval_block(compiler, node->call.args);

	ast_t* call = node->call.callee;
	if(call->class == AST_IDENT)
	{
		emit_invoke(compiler->buffer, call->ident, args);
	}
	else
	{
		// TODO: Subscript?, FIX!!
	}

	return DATA_NULL;
}

datatype_t eval_string(compiler_t* compiler, ast_t* node)
{
	emit_string(compiler->buffer, node->string);
	return DATA_STRING;
}

datatype_t eval_if(compiler_t* compiler, ast_t* node)
{
	// Creates jumps
	// if x == 5
	// 	print 5
	// else if x == 4
	// 	print 4
	// else
	//  print x

	// 1: load x
	// 2: iconst 5
	// 3: ieq			<-- condition 1
	// 4: jmpf 8		<-- jump to condition 2
	// 5: iconst 5
	// 6: print
	// 7: jmp end		<-- end block, jump to end
	// 8: load x 		<-- condition 2
	// 9: iconst 5
	//10: ieq
	//11: jmpf ..		<-- jump to condition 3 and so on

	// Eval the sub-ifclauses
	list_t* jmps = list_new();
	list_iterator_t* iter = list_iterator_create(node->ifstmt);
	while(!list_iterator_end(iter))
	{
		// Get sub-ifclause
		ast_t* subnode = list_iterator_next(iter);

		// Test if not an else-statement
		value_t* instr = 0;
		if(subnode->ifclause.cond)
		{
			// Eval the condition and generate an if-false jump
			compiler_eval(compiler, subnode->ifclause.cond);
			instr = emit_jmpf(compiler->buffer, 0);
		}

		// Eval execution block code
		eval_block(compiler, subnode->ifclause.body);

		// Optimization
		// If not an else statement and more ifclauses than one
		// Generate a jump to end
		if(list_size(node->ifstmt) > 1 && subnode->ifclause.cond)
		{
			value_t* pos = emit_jmp(compiler->buffer, 0);
			list_push(jmps, pos);
		}

		// If not an else statement
		// Generate jump to next clause
		if(subnode->ifclause.cond)
		{
			instr->v.i = list_size(compiler->buffer);
		}
	}
	list_iterator_free(iter);

	// Set the jump points to end after whole if block
	iter = list_iterator_create(jmps);
	while(!list_iterator_end(iter))
	{
		value_t* pos = list_iterator_next(iter);
		pos->v.i = list_size(compiler->buffer);
	}
	list_iterator_free(iter);
	list_free(jmps);

	return DATA_NULL;
}

datatype_t eval_ifclause(compiler_t* compiler, ast_t* node)
{
	// helper function for if
	compiler_eval(compiler, node->ifclause.cond);
	value_t* instr = emit_jmpf(compiler->buffer, 0);
	eval_block(compiler, node->ifclause.body);
	value_set_int(instr, list_size(compiler->buffer));
	return DATA_NULL;
}

datatype_t eval_while(compiler_t* compiler, ast_t* node)
{
	size_t start = list_size(compiler->buffer);
	compiler_eval(compiler, node->whilestmt.cond);
	value_t* instr = emit_jmpf(compiler->buffer, 0);
	eval_block(compiler, node->ifclause.body);
	emit_jmp(compiler->buffer, start);
	value_set_int(instr, list_size(compiler->buffer));
	return DATA_NULL;
}

// Real compile function
datatype_t compiler_eval(compiler_t* compiler, ast_t* node)
{
	if(compiler->error) return DATA_NULL;

	switch(node->class)
	{
		case AST_TOPLEVEL: return eval_block(compiler, node->toplevel);
		case AST_DECLVAR: return eval_declvar(compiler, node);
		case AST_DECLFUNC: return eval_declfunc(compiler, node);
		case AST_FLOAT: return eval_number(compiler, node);
		case AST_INT: return eval_number(compiler, node);
		case AST_STRING: return eval_string(compiler, node);
		case AST_BINARY: return eval_binary(compiler, node);
		case AST_IDENT: return eval_ident(compiler, node);
		case AST_CALL: return eval_call(compiler, node);
		case AST_IF: return eval_if(compiler, node);
		case AST_WHILE: return eval_while(compiler, node);
		default: break;
	}

	return DATA_NULL;
}

/**
 *	'Dumps' the node to the console.
 *	Basically a printing function for abstract syntax trees
 */
void compiler_dump(ast_t* node, int level)
{
	if(!node) return;

	if(level > 0) console("  ");
	for(int i = 0; i < level; i++) console("#");

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
			fprintf(stdout, ":func<%s::(", node->funcdecl.name);

			list_iterator_t* iter = list_iterator_create(node->funcdecl.impl.formals);
			while(!list_iterator_end(iter))
			{
				param_t* param = list_iterator_next(iter);
				fprintf(stdout, "%s: %d", param->name, param->type);

				if(!list_iterator_end(iter))
				{
					fprintf(stdout, ", ");
				}
			}
			fprintf(stdout, ") => ret: %d>\n", node->funcdecl.rettype);

			list_iterator_free(iter);
			iter = list_iterator_create(node->funcdecl.impl.body);
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
		// case AST_ARRAY:
		// {
		// 	fprintf(stdout, ":array<");
		//
		// 	list_iterator_t* iter = list_iterator_create(node->array);
		// 	while(!list_iterator_end(iter))
		// 	{
		// 		compiler_dump(list_iterator_next(iter), 0);
		// 	}
		// 	list_iterator_free(iter);
		// 	fprintf(stdout, ">");
		// 	break;
		// }
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
	if(!cmpl->filename)
	{
		console("Non-files currently not supported");
		return;
	}

	llvm_context_t* ctx = llvm_context_create(cmpl->filename);
	llvm_traverse(ctx, root);

/*	char *error = 0;
	if(LLVMVerifyModule(ctx->module, LLVMAbortProcessAction, &error) != 0)
	{
		LLVMDisposeMessage(error);
	}*/

	if(cmpl->filename)
	{
		fprintf(stdout, "Writing bitcode to file...\n");
		// Write out bitcode to file
	    if (LLVMWriteBitcodeToFile(ctx->module, "out.bc") != 0) {
	        fprintf(stderr, "Error writing bitcode to file, skipping\n");
	    }
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
	// Reset compiler
	compiler_clear(compiler);
	compiler->buffer = list_new();
	compiler->address = 0;
	compiler->symbols = hashmap_new();

	// Run the parser
	parser_init(&compiler->parser);
	ast_t* root = parser_run(&compiler->parser, source);
	if(root)
	{
		if(compiler->debug)
		{
			console("Abstract syntax tree:\n");
			compiler_dump(root, 0);
		}
		compiler_eval(compiler, root);
		//llvm_compile(compiler, root);

		ast_free(root);
	}

	// Free filename and parser
	if(compiler->filename)
	{
		free(compiler->filename);
	}
	parser_free(&compiler->parser);

	// Clean up symbol table / debugging symbols
	compiler_free_symbols(compiler);

	// Return bytecode if valid
	if(root && !compiler->error)
	{
		return compiler->buffer;
	}
	else
	{
		compiler_clear(compiler);
		return 0;
	}
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

	compiler->filename = strdup(filename);

	// run vm with compiler
	list_t* buffer = compile_buffer(compiler, source);
	free(source);

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

void compiler_free_symbols(compiler_t* compiler)
{
	hashmap_iterator_t* iter = hashmap_iterator_create(compiler->symbols);
	while(!hashmap_iterator_end(iter))
	{
		symbol_t* val = hashmap_iterator_next(iter);


		free(val);
	}
	hashmap_iterator_free(iter);
	hashmap_free(compiler->symbols);
}
