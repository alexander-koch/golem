#include "jit.h"

#undef malloc
#undef free
#undef realloc
#undef calloc
#include <windows.h>

#define EAX 0
#define ECX 1
#define EDX 2
#define EBX 3

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
	return VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}

void jit_free_exec(void* ptr, unsigned int size)
{
	if(ptr)
	{
		VirtualFree(ptr, 0, MEM_RELEASE);
	}
}

void jit_x86_64_put(jit_context_t* jit, u8 val, size_t len)
{
	if(jit->size - (jit->ptr - jit->start) < len)
	{
		size_t dist = jit->ptr - jit->start;
		jit->size += 2048;
    	jit->start = (u8*)realloc(jit->start, sizeof(u8) * jit->size);
    	jit->ptr = jit->start + dist;
  	}

	if(len == sizeof(u8))
		*jit->ptr = (u8)val;
  	else if (len == sizeof(int))
    	*((int *)jit->ptr) = (int)val;

  	jit->ptr += len;
}

JitFunc* jit_compile(vector_t* buffer)
{
	jit_context_t ctx;
	jit_context_t* jit = &ctx;

	jit->size = 2048;
	jit->start = jit->ptr = malloc(sizeof(u8) * jit->size);

	// prologue
	// push %ebp
	jit_x86_64_put(jit, 0x55, sizeof(u8));

	// movq %rsp,%rbp
	jit_x86_64_put(jit, 0x48, sizeof(u8));
	jit_x86_64_put(jit, 0x89, sizeof(u8));
	jit_x86_64_put(jit, 0xe5, sizeof(u8));

	u8 reg[] = {EAX, ECX, EDX, EBX};
	u8 rp = 0;

	#define REG_PUSH() reg[rp++]
	#define REG_POP() reg[--rp]
	#define REG_XFER(r1, r2) 0xC0 | r1 << 3 | r2

	for(size_t i = 0; i < vector_size(buffer); i++)
	{
		instruction_t* instr = vector_get(buffer, i);
		switch(instr->op)
		{
			case OP_PUSH:
			{
				if(instr->v1->type == VALUE_INT)
				{
					// mov [int], reg
					jit_x86_64_put(jit, 0xB8 + REG_PUSH(), sizeof(u8));
					jit_x86_64_put(jit, value_int(instr->v1), sizeof(int));
				}
				break;
			}
			case OP_IADD:
			{
				// add r1,r2
				u8 r1 = REG_POP();
				u8 r2 = REG_POP();
				jit_x86_64_put(jit, 0x01, sizeof(u8));
				jit_x86_64_put(jit, REG_XFER(r1, r2), sizeof(u8));
				break;
			}
			case OP_SYSCALL:
			{
				break;
			}
			default: break;
		}
	}

	// epilogue
	// leave
	jit_x86_64_put(jit, 0xC9, sizeof(u8));

	// ret
	jit_x86_64_put(jit, 0xC3, sizeof(u8));

	jit->size = jit->ptr - jit->start;
	JitFunc *func = (JitFunc*)jit_malloc_exec(jit->size);
  	memcpy(func, jit->start, sizeof(u8)*jit->size);

	free(jit->start);
	return func;
}
