#include "vm.h"

value_t* vm_eval(vm_t* vm, ast_t* node);

void vm_init(vm_t* vm)
{
	vm->scope = scope_new();
}

value_t* vm_eval_block(vm_t* vm, list_t* block)
{
	value_t* val = 0;
	list_iterator_t* iter = list_iterator_create(block);
	while(!list_iterator_end(iter))
	{
		ast_t* next = list_iterator_next(iter);
		val = vm_eval(vm, next);
	}
	list_iterator_free(iter);
	return val;
}

value_t* vm_eval_declvar(vm_t* vm, ast_t* node)
{
	value_t* value = vm_eval(vm, node->vardecl.initializer);
	// TODO: Create file and function for scope class (encapsulate)
//	scope_add_var(vm->scope, node->vardecl.name, value);
	value_free(value);

	return 0;// value;
}

value_t* vm_eval_number(vm_t* vm, ast_t* node)
{
	if(node->class == AST_FLOAT)
	{
		value_t* num = value_new_float(node->f);
		return num;
	}
	else
	{
		value_t* num = value_new_float(node->i);
		return num;
	}
}

value_t* vm_eval_binary(vm_t* vm, ast_t* node)
{
	// convert to inverse polish notation
	// push involves evaluation of current ast
	// convert:
	// 	push left
	// 	push op
	//
	// 	if(right == binary)
	// 		convert(binary)
	// 	else
	// 		push right
	//
	// end:
	// 	eval
//	return eval_expression(node);
	return 0;
}

value_t* vm_eval_string(vm_t* vm, ast_t* node)
{
	value_t* str = value_new_string(node->string);
	return str;
}

value_t* vm_eval(vm_t* vm, ast_t* node)
{
	value_t* null = 0;
	switch(node->class)
	{
		case AST_TOPLEVEL: return vm_eval_block(vm, &node->toplevel);
		case AST_DECLVAR: return vm_eval_declvar(vm, node);
		case AST_FLOAT: return vm_eval_number(vm, node);
		case AST_INT: return vm_eval_number(vm, node);
		case AST_STRING: return vm_eval_string(vm, node);
		case AST_BINARY: return vm_eval_binary(vm, node);
		default: return null;
	}
	return null;
}

void vm_dump(ast_t* node)
{
	switch(node->class)
	{
		case AST_TOPLEVEL:
		{
			list_iterator_t* iter = list_iterator_create(&node->toplevel);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				vm_dump(next);
				fprintf(stdout, "\n");
			}
			list_iterator_free(iter);
			break;
		}
		case AST_DECLVAR:
		{
			fprintf(stdout, ":decl %s<", node->vardecl.name);
			vm_dump(node->vardecl.initializer);
			fprintf(stdout, ">");
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
			fprintf(stdout, ":num = %li", node->i);
			break;
		}
		case AST_STRING:
		{
			fprintf(stdout, ":str = '%s'", node->string);
			break;
		}
		case AST_BINARY:
		{
			fprintf(stdout, ":bin<");
			vm_dump(node->binary.left);
			vm_dump(node->binary.right);
			fprintf(stdout, ":op = %d>", node->binary.op);
			break;
		}
		case AST_CALL:
		{
			fprintf(stdout, ":call<");
			vm_dump(node->call.callee);
			fprintf(stdout, ">");
			break;
		}
		default: break;
	}
}

void vm_run_buffer(vm_t* vm, const char* source)
{
	parser_init(&vm->parser);
	ast_t* root = parser_run(&vm->parser, source);
	if(root)
	{
		vm_dump(root);
		// vm_eval(vm, root);

		ast_free(root);
	}

	parser_free(&vm->parser);
}

int vm_run_file(vm_t* vm, const char* filename)
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
	vm_run_buffer(vm, source);
	free(source);
	return 1;
}

void vm_free(vm_t* vm)
{
	scope_free(vm->scope);
}
