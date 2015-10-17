#include "graphviz.h"

void graphviz_mnemonic(graphviz_t* state)
{
	for(size_t i = 0; i < state->mnemonic; i++)
	{
		fprintf(state->fp, " ");
	}
}

void graphviz_mnemonic_inc(graphviz_t* state)
{
	state->mnemonic += 4;
}

void graphviz_mnemonic_dec(graphviz_t* state)
{
	state->mnemonic -= 4;
}

int graphviz_get_id(graphviz_t* state)
{
	return state->id++;
}

void graphviz_connection(graphviz_t* state, int v1, int v2)
{
	graphviz_mnemonic(state);
	fprintf(state->fp, "node%d -> node%d\n", v1, v2);
}

int graphviz_eval(graphviz_t* state, ast_t* node)
{
	switch(node->class)
	{
		case AST_NULL: return 0;
		case AST_IDENT:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"IDENT %s\"]\n", this, node->ident);
			return this;
		}
		case AST_FLOAT:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"FLOAT %f\"]\n", this, (float)node->f);
			return this;
		}
		case AST_INT:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"INT %d\"]\n", this, (int)node->i);
			return this;
		}
		case AST_BOOL:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"BOOL %s\"]\n", this, node->b ? "true" : "false");
			return this;
		}
		case AST_STRING:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"STRING %s\"]\n", this, node->string);
			return this;
		}
		case AST_CHAR:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"CHAR %c\"]\n", this, node->ch);
			return this;
		}
		case AST_ARRAY:
		{
			int this = graphviz_get_id(state);

			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"ARRAY\"]\n", this);

			list_iterator_t* iter = list_iterator_create(node->array.elements);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				int other = graphviz_eval(state, next);
				graphviz_connection(state, this, other);
			}
			list_iterator_free(iter);
			return this;
		}
		case AST_BINARY:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"BINARY %s\"]\n", this, tok2str(node->binary.op));

			int leftId = graphviz_eval(state, node->binary.left);
			int rightId = graphviz_eval(state, node->binary.right);
			graphviz_connection(state, this, leftId);
			graphviz_connection(state, this, rightId);
			return this;
		}
		case AST_UNARY:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"UNARY %s\"]\n", this, tok2str(node->unary.op));
			int other = graphviz_eval(state, node->unary.expr);
			graphviz_connection(state, this, other);
			return this;
		}
		case AST_SUBSCRIPT:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"SUBSCRIPT (K/EXPR)\"]\n", this);

			int keyId = graphviz_eval(state, node->subscript.key);
			graphviz_connection(state, this, keyId);

			int exprId = graphviz_eval(state, node->subscript.expr);
			graphviz_connection(state, this, exprId);
			return this;
		}
		case AST_CALL:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"CALL\"]\n", this);

			int other = graphviz_eval(state, node->call.callee);
			graphviz_connection(state, this, other);

			// Get the arguments / parameters
			int args = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"ARGS\"]\n", args);

			list_iterator_t* iter = list_iterator_create(node->call.args);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				other = graphviz_eval(state, next);
				graphviz_connection(state, args, other);
			}
			list_iterator_free(iter);

			graphviz_connection(state, this, args);
			return this;
		}
		case AST_DECLVAR:
		{
			int this = graphviz_get_id(state);

			char* name = node->vardecl.name;
			ast_t* initializer = node->vardecl.initializer;
			bool mutate = node->vardecl.mutate;
			datatype_t type = node->vardecl.type;

			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"DECLVAR %s\\nmutate:%s\\ntype:%s\", shape=\"record\"]\n", this, name, mutate?"true":"false", datatype2str(type));

			if(initializer)
			{
				int other = graphviz_eval(state, initializer);
				graphviz_connection(state, this, other);
			}

			return this;
		}
		case AST_DECLFUNC:
		{
			int this = graphviz_get_id(state);
			char* name = node->funcdecl.name;
			datatype_t ret = node->funcdecl.rettype;
			bool ext = node->funcdecl.external;

			if(!strcmp(name, "println") || !strcmp(name, "print") ||
				!strcmp(name, "break") || !strcmp(name, "getline") || !strcmp(name, "parseFloat"))
			{
				return 0;
			}

			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"DECLFUNC %s\\ntype:%s\\nexternal:%s\", shape=\"record\"]\n", this, name, datatype2str(ret), ext?"true":"false");

			// Get the formals
			int formalsId = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"FORMALS\"]\n", formalsId);

			list_iterator_t* iter = list_iterator_create(node->funcdecl.impl.formals);
			while(!list_iterator_end(iter))
			{
				ast_t* param = list_iterator_next(iter);
				int other = graphviz_eval(state, param);
				graphviz_connection(state, formalsId, other);
			}
			list_iterator_free(iter);
			graphviz_connection(state, this, formalsId);

			// Body
			int bodyId = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"BODY\"]\n", bodyId);
			iter = list_iterator_create(node->funcdecl.impl.body);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				int other = graphviz_eval(state, next);
				graphviz_connection(state, bodyId, other);
			}
			list_iterator_free(iter);
			graphviz_connection(state, this, bodyId);

			return this;
		}
		case AST_IF:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"IF\"]\n", this);

			list_iterator_t* iter = list_iterator_create(node->ifstmt);
			while(!list_iterator_end(iter))
			{
				ast_t* subclause = list_iterator_next(iter);
				int other = graphviz_eval(state, subclause);
				graphviz_connection(state, this, other);
			}
			list_iterator_free(iter);
			return this;
		}
		case AST_IFCLAUSE:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);

			if(node->ifclause.cond)
			{
				fprintf(state->fp, "node%d [label=\"IFCLAUSE\"]\n", this);

				int condBranch = graphviz_get_id(state);
				graphviz_mnemonic(state);
				fprintf(state->fp, "node%d [label=\"COND\"]\n", condBranch);

				int cond = graphviz_eval(state, node->ifclause.cond);
				graphviz_connection(state, condBranch, cond);
				graphviz_connection(state, this, condBranch);
			}
			else
			{
				fprintf(state->fp, "node%d [label=\"ELSE\"]\n", this);
			}

			int bodyBranch = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"BODY\"]\n", bodyBranch);

			list_iterator_t* iter = list_iterator_create(node->ifclause.body);
			while(!list_iterator_end(iter))
			{
				ast_t* subnode = list_iterator_next(iter);
				int other = graphviz_eval(state, subnode);
				graphviz_connection(state, bodyBranch, other);
			}
			list_iterator_free(iter);
			graphviz_connection(state, this, bodyBranch);

			return this;
		}
		case AST_WHILE:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"WHILE\"]\n", this);

			int condBranch = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"COND\"]\n", condBranch);

			int cond = graphviz_eval(state, node->whilestmt.cond);
			graphviz_connection(state, condBranch, cond);
			graphviz_connection(state, this, condBranch);

			int bodyBranch = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"BODY\"]\n", bodyBranch);

			list_iterator_t* iter = list_iterator_create(node->whilestmt.body);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				int other = graphviz_eval(state, next);
				graphviz_connection(state, bodyBranch, other);
			}
			list_iterator_free(iter);
			graphviz_connection(state, this, bodyBranch);
			return this;
		}
		case AST_IMPORT:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"IMPORT %s\"]\n", this, node->import);
			return this;
		}
		case AST_CLASS:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"CLASS %s\"]\n", this, node->classstmt.name);

			int formalBranch = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"FORMALS\"]\n", formalBranch);

			list_iterator_t* iter = list_iterator_create(node->classstmt.formals);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				int other = graphviz_eval(state, next);
				graphviz_connection(state, formalBranch, other);
			}
			list_iterator_free(iter);
			graphviz_connection(state, this, formalBranch);

			int bodyBranch = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"BODY\"]\n", bodyBranch);

			iter = list_iterator_create(node->classstmt.body);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);
				int other = graphviz_eval(state, next);
				graphviz_connection(state, bodyBranch, other);
			}
			list_iterator_free(iter);
			graphviz_connection(state, this, bodyBranch);
			return this;
		}
		case AST_RETURN:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"RETURN\"]\n", this);

			if(node->returnstmt)
			{
				int other = graphviz_eval(state, node->returnstmt);
				graphviz_connection(state, this, other);
			}

			return this;
		}
		case AST_TOPLEVEL:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"ROOT\"]\n", this);

			list_iterator_t* iter = list_iterator_create(node->toplevel);
			while(!list_iterator_end(iter))
			{
				ast_t* next = list_iterator_next(iter);

				int child = graphviz_eval(state, next);
				if(child != 0) graphviz_connection(state, this, child);
			}
			list_iterator_free(iter);
			return this;
		}
		case AST_ANNOTATION:
		{
			int this = graphviz_get_id(state);
			graphviz_mnemonic(state);
			fprintf(state->fp, "node%d [label=\"ANNOTATION %d\"]\n", this, (int)node->annotation);
			return this;
		}
		default: break;
	}

	return 0;
}

void graphviz_build(ast_t* root)
{
	FILE* fp = fopen("ast.dot", "wb");
	fprintf(fp, "digraph astgraph {\n");
	//fprintf(fp, "    graph [rankdir=\"LR\"];\n"); // <-- Left / right or top / down
	fprintf(fp, "    node [shape=ellipse, fontsize=14, fontname=\"Courier\", height=.1];\n");
	fprintf(fp, "    ranksep=.8;\n");
	//fprintf(fp, "    edge [arrowsize=.5]\n\n");

	graphviz_t state;
	state.fp = fp;
	state.root = root;
	state.mnemonic = 4;
	state.id = 1;
	graphviz_eval(&state, root);

	fprintf(fp, "}\n");
	fclose(fp);
}
