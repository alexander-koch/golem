#include "ast.h"

const char* datatype2str(datatype_t type) {
	switch(type.type) {
		case DATA_NULL: return "null";
		case DATA_BOOL: return "bool";
		case DATA_INT: return "int";
		case DATA_FLOAT: return "float";
		case DATA_CHAR: return "char";
		case DATA_CLASS: return "type";
		case DATA_VOID: return "void";
		case DATA_ARRAY: return "null-array";
		case DATA_GENERIC: return "generic";
		case DATA_LAMBDA: return "lambda";
		default: {
			if((type.type & DATA_ARRAY) == DATA_ARRAY) {
				int subtype = type.type & ~DATA_ARRAY;
				switch(subtype) {
					case DATA_CHAR: return "char[]";
					case DATA_INT: return "int[]";
					case DATA_BOOL: return "bool[]";
					case DATA_FLOAT: return "float[]";
					case DATA_CLASS: return "class[]";
					case DATA_VOID: return "void[]";
					case DATA_GENERIC: return "generic[]";
					default: return "null[]";
				}
			}

			return "null";
		}
	}
}

ast_t* ast_class_create(ast_class_t class, location_t location) {
	ast_t* ast = calloc(1, sizeof(*ast));
	if(!ast) return ast;
	ast->class = class;
	ast->location = location;
    return ast;
}

const char* ast_classname(ast_class_t class) {
	switch(class) {
		case AST_NULL: return "null";
		case AST_IDENT: return "identifier";
		case AST_FLOAT: return "float";
		case AST_INT: return "integer";
		case AST_BOOL: return "bool";
		case AST_STRING: return "string";
		case AST_CHAR: return "char";
		case AST_ARRAY: return "array";
		case AST_BINARY: return "binary";
		case AST_UNARY: return "unary";
		case AST_SUBSCRIPT: return "subscript";
		case AST_CALL: return "call";
		case AST_DECLVAR: return "variable declaration";
		case AST_DECLFUNC: return "function declaration";
		case AST_IF: return "if condition";
		case AST_IFCLAUSE: return "if clause";
		case AST_WHILE: return "while loop";
		case AST_IMPORT: return "import";
		case AST_CLASS: return "class";
		case AST_RETURN: return "return";
		case AST_TOPLEVEL: return "toplevel";
		case AST_ANNOTATION: return "annotation";
		default: return "null";
	}
}

void ast_free(ast_t* ast) {
	if(!ast) return;

	list_iterator_t* iter = 0;
	switch(ast->class) {
		case AST_TOPLEVEL: {
			iter = list_iterator_create(ast->toplevel);
			while(!list_iterator_end(iter)) {
				ast_free(list_iterator_next(iter));
			}
			list_iterator_free(iter);
			list_free(ast->toplevel);
			break;
		}
		case AST_DECLVAR: {
			ast_free(ast->vardecl.initializer);
			break;
		}
		case AST_DECLFUNC: {
			iter = list_iterator_create(ast->funcdecl.impl.body);
			while(!list_iterator_end(iter)) {
				ast_free(list_iterator_next(iter));
			}
			list_iterator_free(iter);
			list_free(ast->funcdecl.impl.body);

			iter = list_iterator_create(ast->funcdecl.impl.formals);
			while(!list_iterator_end(iter)) {
				ast_t* param = list_iterator_next(iter);
				ast_free(param);
			}
			list_iterator_free(iter);
			list_free(ast->funcdecl.impl.formals);

			if(ast->funcdecl.external) {
				free(ast->funcdecl.name);
			}
			break;
		}
		case AST_BINARY: {
			ast_free(ast->binary.left);
			ast_free(ast->binary.right);
			break;
		}
		case AST_UNARY: {
			ast_free(ast->unary.expr);
			break;
		}
		case AST_ARRAY: {
			iter = list_iterator_create(ast->array.elements);
			while(!list_iterator_end(iter)) {
				ast_free(list_iterator_next(iter));
			}
			list_iterator_free(iter);
			list_free(ast->array.elements);
			break;
		}
		case AST_SUBSCRIPT: {
			ast_free(ast->subscript.key);
			ast_free(ast->subscript.expr);
			break;
		}
		case AST_IF: {
			iter = list_iterator_create(ast->ifstmt);
			while(!list_iterator_end(iter)) {
				ast_free(list_iterator_next(iter));
			}
			list_iterator_free(iter);
			list_free(ast->ifstmt);
			break;
		}
		case AST_IFCLAUSE: {
			iter = list_iterator_create(ast->ifclause.body);
			while(!list_iterator_end(iter)) {
				ast_free(list_iterator_next(iter));
			}
			list_iterator_free(iter);
			list_free(ast->ifclause.body);
			ast_free(ast->ifclause.cond);
			break;
		}
		case AST_WHILE: {
			iter = list_iterator_create(ast->whilestmt.body);
			while(!list_iterator_end(iter)) {
				ast_free(list_iterator_next(iter));
			}
			list_iterator_free(iter);
			list_free(ast->whilestmt.body);
			ast_free(ast->whilestmt.cond);
			break;
		}
		case AST_CLASS: {
			iter = list_iterator_create(ast->classstmt.body);
			while(!list_iterator_end(iter)) {
				ast_free(list_iterator_next(iter));
			}
			list_iterator_free(iter);
			list_free(ast->classstmt.body);

			iter = list_iterator_create(ast->classstmt.formals);
			while(!list_iterator_end(iter)) {
				ast_free(list_iterator_next(iter));
			}
			list_iterator_free(iter);
			list_free(ast->classstmt.formals);

			hashmap_free(ast->classstmt.fields);
			break;
		}
		case AST_RETURN: {
			ast_free(ast->returnstmt);
			break;
		}
		case AST_CALL: {
			ast_free(ast->call.callee);

			iter = list_iterator_create(ast->call.args);
			while(!list_iterator_end(iter)) {
				ast_free(list_iterator_next(iter));
			}
			list_iterator_free(iter);
			list_free(ast->call.args);
			break;
		}
		default: break;
	}
	free(ast);
}

/**
 * ast_dump:
 * 'Dumps' the node to the console.
 * Prints the abstract syntax tree.
 */
void ast_dump(ast_t* node, int level) {
#ifndef NO_AST
	if(!node) return;

	for(int i = 0; i < level; i++) printf("  ");

	switch(node->class) {
		case AST_TOPLEVEL: {
			list_iterator_t* iter = list_iterator_create(node->toplevel);
			while(!list_iterator_end(iter)) {
				ast_t* next = list_iterator_next(iter);
				ast_dump(next, level+1);
				putchar('\n');
			}
			list_iterator_free(iter);
			break;
		}
		case AST_ANNOTATION: {
			printf("(annotation %d)", (int)node->annotation);
			break;
		}
		case AST_DECLVAR: {
			printf("(var name='%s' type=%s ID=%lu", node->vardecl.name, datatype2str(node->vardecl.type), node->vardecl.type.id);
			if(node->vardecl.initializer) {
				putchar('\n');
				ast_dump(node->vardecl.initializer, level+1);
			}
			putchar(')');
			break;
		}
		case AST_DECLFUNC: {
			printf("(func name='%s' type=%s ID=%lu params = {", node->funcdecl.name, datatype2str(node->funcdecl.rettype), node->funcdecl.rettype.id);

			list_iterator_t* iter = list_iterator_create(node->funcdecl.impl.formals);
			while(!list_iterator_end(iter)) {
				ast_t* param = list_iterator_next(iter);
				printf("%s: %s->%lu", param->vardecl.name, datatype2str(param->vardecl.type), param->vardecl.type.id);

				if(!list_iterator_end(iter)) {
					printf(", ");
				}
			}
			putchar('}');
			list_iterator_free(iter);

			if(node->funcdecl.impl.body) {
				putchar('\n');
				iter = list_iterator_create(node->funcdecl.impl.body);
				while(!list_iterator_end(iter)) {
					ast_t* next = list_iterator_next(iter);
					ast_dump(next, level+1);
					if(!list_iterator_end(iter)) putchar('\n');
				}
				list_iterator_free(iter);
			}
			putchar(')');
			break;
		}
		case AST_IDENT: {
			printf("(ident = %s)", node->ident);
			break;
		}
		case AST_FLOAT: {
			printf("(num = %f)", node->f);
			break;
		}
		case AST_INT: {
			printf("(num = %d)", node->i);
			break;
		}
		case AST_STRING: {
			printf("(str = '%s')", node->string);
			break;
		}
		case AST_CHAR: {
			printf("(char = '%c')", node->ch);
			break;
		}
		case AST_BOOL: {
			printf("(bool = %s)", node->b == true ? "true" : "false");
			break;
		}
		case AST_BINARY: {
			printf("(bin op=%s\n", tok2str(node->binary.op));
			ast_dump(node->binary.left, level+1);
			putchar('\n');
			ast_dump(node->binary.right, level+1);
			putchar(')');
			break;
		}
		case AST_UNARY: {
			printf("(unary<%s, ", tok2str(node->unary.op));
			ast_dump(node->unary.expr, 0);
			putchar(')');
			break;
		}
		case AST_SUBSCRIPT: {
			printf("(subscript\n");
			ast_dump(node->subscript.key, level+1);
			printf("\n");
			ast_dump(node->subscript.expr, level+1);
			putchar(')');
			break;
		}
		case AST_IF: {
			printf("(if\n");
			list_iterator_t* iter = list_iterator_create(node->ifstmt);
			while(!list_iterator_end(iter)) {
				ast_t* next = list_iterator_next(iter);
				ast_dump(next, level+1);
				if(!list_iterator_end(iter)) putchar('\n');
			}
			list_iterator_free(iter);
			putchar(')');
			break;
		}
		case AST_IFCLAUSE: {
			if(node->ifclause.cond) {
				printf("(ifclause\n");
				for(int i = 0; i < level+2; i++) printf("  ");
				printf("(cond\n");
				ast_dump(node->ifclause.cond, level+3);
				printf(")\n");
			}
			else {
				printf("(else\n");
			}

			if(list_size(node->ifclause.body) > 0) {
				for(int i = 0; i < level+2; i++) printf("  ");
				printf("(body\n");
				list_iterator_t* iter = list_iterator_create(node->ifclause.body);
				while(!list_iterator_end(iter)) {
					ast_t* next = list_iterator_next(iter);
					ast_dump(next, level+3);
					if(!list_iterator_end(iter)) putchar('\n');
				}
				list_iterator_free(iter);
				putchar(')');
			}
			putchar(')');
			break;
		}
		case AST_WHILE: {
			printf("(while\n");
			ast_dump(node->whilestmt.cond, level+1);
			printf(")\n");

			list_iterator_t* iter = list_iterator_create(node->whilestmt.body);
			while(!list_iterator_end(iter)) {
				ast_t* next = list_iterator_next(iter);
				ast_dump(next, level+1);
				if(!list_iterator_end(iter)) putchar('\n');
			}
			list_iterator_free(iter);
			putchar(')');
			break;
		}
		case AST_IMPORT: {
			printf("(import %s)", node->import);
			break;
		}
		case AST_ARRAY: {
			printf("(array\n");

			list_iterator_t* iter = list_iterator_create(node->array.elements);
			while(!list_iterator_end(iter)) {
				ast_dump(list_iterator_next(iter), level+1);
				if(!list_iterator_end(iter)) putchar('\n');
			}
			list_iterator_free(iter);
			putchar(')');
			break;
		}
		case AST_CLASS: {
			printf("(class name='%s' ID=%lu\n", node->classstmt.name, djb2((unsigned char*)node->classstmt.name));

			list_iterator_t* iter = list_iterator_create(node->classstmt.formals);
			while(!list_iterator_end(iter)) {
				ast_t* next = list_iterator_next(iter);
				ast_dump(next, level+1);
				putchar('\n');
			}
			list_iterator_free(iter);

			iter = list_iterator_create(node->classstmt.body);
			while(!list_iterator_end(iter)) {
				ast_t* next = list_iterator_next(iter);
				ast_dump(next, level+1);
				list_iterator_end(iter) ? printf(")\n") : putchar('\n');
			}
			list_iterator_free(iter);
			break;
		}
		case AST_RETURN: {
			printf("(return\n");
			ast_dump(node->returnstmt, level+1);
			putchar(')');
			break;
		}
		case AST_CALL: {
			printf("(call\n");
			ast_dump(node->call.callee, level+1);

			// list_iterator_t* iter = list_iterator_create(node->call.args);
			// while(!list_iterator_end(iter))
			// {
			// 	ast_t* next = list_iterator_next(iter);
			// 	ast_dump(next, level);
			// }
			// list_iterator_free(iter);
			putchar(')');
			break;
		}
		default: break;
	}
#endif
}
