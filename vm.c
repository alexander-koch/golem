#include "vm.h"

// int vm_eval_block(list_t* block)
// {
// 	list_iterator_t* iter = list_iterator_create(block);
// 	while(!list_iterator_end(iter))
// 	{
// 		ast_t* next = list_iterator_next(iter);
// 	}
// 	list_iterator_free(iter);
// }
//
// int vm_eval(ast_t* node)
// {
// 	switch(node->class)
// 	{
// 		case AST_TOPLEVEL: return vm_eval_block(node->toplevel);
// 		case AST_DECLVAR: return vm_eval_statement(node);
// 		default: return 0;
// 	}
// }

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
			}
			list_iterator_free(iter);
			break;
		}
		case AST_DECLVAR:
		{
			fprintf(stdout, "<decl %s", node->vardecl.name);
			vm_dump(node->vardecl.initializer);
			fprintf(stdout, ">\n");
			break;
		}
		case AST_IDENT:
		{
			fprintf(stdout, ":ident = %s", node->ident);
			break;
		}
		case AST_NUMBER:
		{
			fprintf(stdout, ":num = %f", node->number);
			break;
		}
		case AST_STRING:
		{
			fprintf(stdout, ":str = '%s'", node->string);
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
		// TODO: Eval abstract syntax tree
		vm_dump(root);
	}
	else
	{
		// TODO: Throw error
	}

	ast_free(root);
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
