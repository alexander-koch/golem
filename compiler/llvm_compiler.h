#ifndef llvm_compiler_h
#define llvm_compiler_h

#include <limits.h>
#include <parser/ast.h>
#include <adt/hashmap.h>

#ifdef __USE_LLVM__
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

typedef struct
{
	LLVMModuleRef module;
	LLVMBuilderRef builder;
	hashmap_t* variables;
} llvm_context_t;

llvm_context_t* llvm_context_create(const char* name);
LLVMValueRef llvm_traverse(llvm_context_t* ctx, ast_t* node);
void llvm_context_free(llvm_context_t* ctx);

#endif

#endif
