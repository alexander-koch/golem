#ifndef doc_h
#define doc_h

#include <compiler/compiler.h>
#include <compiler/graphviz.h>
#include <vm/bytecode.h>
#include <adt/vector.h>

void doc_generate(compiler_t* compiler, const char* filename);

#endif
