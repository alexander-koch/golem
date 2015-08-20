#ifndef jit_h
#define jit_h

#include <adt/vector.h>
#include <vm/bytecode.h>

typedef int (JitFunc)();
typedef unsigned char u8;

typedef struct
{
	u8* start;
	u8* ptr;
	long size;
} jit_context_t;

void* jit_malloc_exec(unsigned int size);
void jit_free_exec(void* ptr, unsigned int size);

JitFunc* jit_compile(vector_t* buffer);

#endif
