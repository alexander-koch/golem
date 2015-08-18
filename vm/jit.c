#include "jit.h"

#undef malloc
#undef free
#undef realloc
#undef calloc
#include <windows.h>

// X86_64 64 bit general purpose integer registers.
typedef enum
{
	X86_64_RAX = 0,
	X86_64_RCX = 1,
	X86_64_RDX = 2,
	X86_64_RBX = 3,
	X86_64_RSP = 4,
	X86_64_RBP = 5,
	X86_64_RSI = 6,
	X86_64_RDI = 7,
	X86_64_R8 = 8,
	X86_64_R9 = 9,
	X86_64_R10 = 10,
	X86_64_R11 = 11,
	X86_64_R12 = 12,
	X86_64_R13 = 13,
	X86_64_R14 = 14,
	X86_64_R15 = 15,
	X86_64_RIP = 16
} X86_64_Reg;

// X86-64 xmm registers.
typedef enum
{
	X86_64_XMM0 = 0,
	X86_64_XMM1 = 1,
	X86_64_XMM2 = 2,
	X86_64_XMM3 = 3,
	X86_64_XMM4 = 4,
	X86_64_XMM5 = 5,
	X86_64_XMM6 = 6,
	X86_64_XMM7 = 7,
	X86_64_XMM8 = 8,
	X86_64_XMM9 = 9,
	X86_64_XMM10 = 10,
	X86_64_XMM11 = 11,
	X86_64_XMM12 = 12,
	X86_64_XMM13 = 13,
	X86_64_XMM14 = 14,
	X86_64_XMM15 = 15
} X86_64_XMM_Reg;

void* jit_malloc_exec(unsigned int size)
{
// #if defined(JIT_WIN32_PLATFORM)
	return VirtualAlloc(0, size,
				MEM_COMMIT | MEM_RESERVE,
			    PAGE_EXECUTE_READWRITE);
// #elif defined(JIT_USE_MMAP)
// 	void *ptr = mmap(0, size,
// 			 PROT_READ | PROT_WRITE | PROT_EXEC,
// 			 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
// 	if(ptr == (void *)-1)
// 	{
// 		return (void *)0;
// 	}
// 	return ptr;
// #else
// 	return malloc(size);
// #endif
}

void jit_free_exec(void* ptr, unsigned int size)
{
	if(ptr)
	{
	// #if defined(JIT_WIN32_PLATFORM)
		VirtualFree(ptr, 0, MEM_RELEASE);
	// #elif defined(JIT_USE_MMAP)
	// 	munmap(ptr, size);
	// #else
	// 	free(ptr);
	// #endif
	}
}

JitFunc* jit_compile(list_t* buffer)
{
	jit_context_t ctx;
	jit_context_t* jit = &ctx;

	jit->size = 2048;
	jit->start = jit->ptr = malloc(jit->size);


	// Write code
	list_iterator_t* iter = list_iterator_create(buffer);
	while(!list_iterator_end(iter))
	{
		instruction_t* instr = list_iterator_next(iter);
		switch(instr->op)
		{
			case OP_PUSH:
			{
				if(instr->v1->type == VALUE_INT)
				{
					
				}

				break;
			}
			default: break;
		}
	}
	list_iterator_free(iter);

	jit->size = jit->ptr - jit->start;
	JitFunc *func = (JitFunc*)jit_malloc_exec(jit->size);
  	memcpy(func, jit->start, sizeof(u8)*jit->size);

	free(jit->start);
	return func;
}
