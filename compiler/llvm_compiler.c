#include "llvm_compiler.h"
#ifdef __USE_LLVM__

LLVMValueRef llvm_toplevel(llvm_context_t* ctx, ast_t* node);
LLVMValueRef llvm_function(llvm_context_t* ctx, ast_t* node);
LLVMValueRef llvm_var(llvm_context_t* ctx, ast_t* node);
LLVMValueRef llvm_binary(llvm_context_t* ctx, ast_t* node);
LLVMValueRef llvm_string(llvm_context_t* ctx, ast_t* node);
LLVMValueRef llvm_int(llvm_context_t* ctx, ast_t* node);
LLVMValueRef llvm_ident(llvm_context_t* ctx, ast_t* node);
LLVMValueRef llvm_call(llvm_context_t* ctx, ast_t* node);

LLVMTypeRef getLLVMType(datatype_t dt)
{
	switch(dt)
	{
		case DATA_FLOAT: return LLVMDoubleType();
		case DATA_INT: return LLVMInt32Type();
		case DATA_BOOL: return LLVMInt1Type();
		case DATA_VOID: return LLVMVoidType();
		default: break;
	}

	if((dt & DATA_ARRAY) == DATA_ARRAY)
	{
		dt = dt & ~DATA_ARRAY; // Remove bitflag to get type of array
		switch(dt)
		{
			case DATA_INT:
			{
				return LLVMPointerType(LLVMInt32Type(), 0);
				break;
			}
			case DATA_STRING:
			{
				return LLVMPointerType(LLVMInt8Type(), 0);
				break;
			}
			default: break;
		}
	}

	return LLVMInt1Type();
}

llvm_context_t* llvm_context_create(const char* name)
{
	llvm_context_t* ctx = malloc(sizeof(*ctx));
	ctx->module = LLVMModuleCreateWithName(name);
	ctx->builder = LLVMCreateBuilder();
	ctx->variables = hashmap_new();
	return ctx;
}

LLVMValueRef llvm_traverse(llvm_context_t* ctx, ast_t* node)
{
	switch(node->class)
	{
		case AST_TOPLEVEL: return llvm_toplevel(ctx, node);
		case AST_DECLFUNC: return llvm_function(ctx, node);
		case AST_DECLVAR: return llvm_var(ctx, node);
		case AST_BINARY: return llvm_binary(ctx, node);
		case AST_STRING: return llvm_string(ctx, node);
		case AST_INT: return llvm_int(ctx, node);
		case AST_IDENT: return llvm_ident(ctx, node);
		case AST_CALL: return llvm_call(ctx, node);
		default: return 0;
	}
}

void llvm_context_free(llvm_context_t* ctx)
{
	LLVMDisposeBuilder(ctx->builder);
	LLVMDisposeModule(ctx->module);
	hashmap_free(ctx->variables);
	free(ctx);
}

LLVMValueRef llvm_toplevel(llvm_context_t* ctx, ast_t* node)
{
	list_iterator_t* iter = list_iterator_create(node->toplevel);
	LLVMValueRef ref = 0;
	while(!list_iterator_end(iter))
	{
		ast_t* ast = list_iterator_next(iter);
		ref = llvm_traverse(ctx, ast);
	}
	list_iterator_free(iter);
	return ref;
}

LLVMValueRef llvm_function(llvm_context_t* ctx, ast_t* node)
{
	char* name = node->funcdecl.name;
	list_t* param_list = node->funcdecl.impl.formals;
	size_t argc = list_size(param_list);
	LLVMTypeRef* params = 0;

	LLVMValueRef func = LLVMGetNamedFunction(ctx->module, name);
	if(func)
	{
		if(LLVMCountParams(func) != argc)
		{
			printf("Function already defined with differend parameters\n");
			return 0;
		}
		else
		{
			printf("Function redefinition\n");
			return 0;
		}
	}
	else
	{
		params = malloc(sizeof(LLVMTypeRef) * argc);
		for(int i = 0; i < argc; i++)
		{
			param_t* param = list_get(param_list, i);
			params[i] = getLLVMType(param->type);
			// is pointer? LLVMPointerType(insert type here, e.g. LLVMInt32Type());

			// TODO: Pointers
		}

		LLVMTypeRef funcTypeRef = LLVMFunctionType(getLLVMType(node->funcdecl.rettype), params, argc, false);
		func = LLVMAddFunction(ctx->module, name, funcTypeRef);

		// Set param names
		for(int i = 0; i < argc; i++)
		{
			param_t* param = list_get(param_list, i);
			LLVMValueRef vparam = LLVMGetParam(func, i);
			LLVMSetValueName(vparam, param->name);
		}
	}

	// If valid append statements
	if(func)
	{
		LLVMBasicBlockRef block = LLVMAppendBasicBlock(func, "entry");
		LLVMPositionBuilderAtEnd(ctx->builder, block);

		list_iterator_t* iter = list_iterator_create(node->funcdecl.impl.body);
		while(!list_iterator_end(iter))
		{
			ast_t* ast = list_iterator_next(iter);
			llvm_traverse(ctx, ast);
		}
		list_iterator_free(iter);
	}

	free(params);
	return func;
}

LLVMValueRef llvm_var(llvm_context_t* ctx, ast_t* node)
{
	// Save variable and alloca in llvm and also in variable ctx
	LLVMValueRef expr = llvm_traverse(ctx, node->vardecl.initializer);
	LLVMValueRef str = LLVMBuildAlloca(ctx->builder, LLVMTypeOf(expr), node->vardecl.name);
	LLVMBuildStore(ctx->builder, expr, str);

	hashmap_set(ctx->variables, node->vardecl.name, str);
	return str;
}

LLVMValueRef llvm_binary(llvm_context_t* ctx, ast_t* node)
{
	LLVMValueRef lhs = llvm_traverse(ctx, node->binary.left);
	LLVMValueRef rhs = llvm_traverse(ctx, node->binary.right);
	if(!lhs || !rhs)
	{
		// TODO: propely implement
		printf("Invalid binary statement\n");
		return 0;
	}

	bool floating = node->binary.left->class == AST_FLOAT &&
		node->binary.right->class == AST_FLOAT;
	switch(node->binary.op)
	{
		case TOKEN_ADD: return floating ? LLVMBuildFAdd(ctx->builder, lhs, rhs, "add") :
			LLVMBuildAdd(ctx->builder, lhs, rhs, "add");
		case TOKEN_SUB: return floating ? LLVMBuildFSub(ctx->builder, lhs, rhs, "sub") :
			LLVMBuildSub(ctx->builder, lhs, rhs, "sub");
		default: break;
	}

	return lhs;
}

LLVMValueRef llvm_string(llvm_context_t* ctx, ast_t* node)
{
	LLVMValueRef str = LLVMConstString(node->string, strlen(node->string), false);
	return str;
}

LLVMValueRef llvm_int(llvm_context_t* ctx, ast_t* node)
{
	LLVMValueRef integer = LLVMConstInt(LLVMInt32Type(), node->i, false);
	return integer;
}

LLVMValueRef llvm_ident(llvm_context_t* ctx, ast_t* node)
{
	// Load a variable onto the stack
	void* val = 0;
	if(hashmap_get(ctx->variables, node->ident, &val) != HMAP_MISSING)
	{
		LLVMValueRef res = (LLVMValueRef)val;
		return LLVMBuildLoad(ctx->builder, res, node->ident);
	}
	return 0;
}

LLVMValueRef llvm_call(llvm_context_t* ctx, ast_t* node)
{

	return 0;
}

#endif
