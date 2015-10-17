#ifndef graphiz_h
#define graphiz_h

#include <stdio.h>
#include <stdarg.h>
#include <parser/ast.h>

typedef struct
{
	FILE* fp;
	ast_t* root;
	int mnemonic;
	int id;
} graphviz_t;

void graphviz_build(ast_t* root);

#endif
