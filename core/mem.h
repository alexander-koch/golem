#ifndef NO_MEMINFO
#ifdef USE_MEM_IMPLEMENTATION
#undef USE_MEM_IMPLEMENTATION

#ifdef malloc
#undef malloc
#undef free
#undef realloc
#endif

static unsigned long bytes = 0;
static unsigned long used_bytes = 0;
static int allocs = 0;
static int frees = 0;

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <malloc.h>

void mem_error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Fatal error: ");
    vfprintf(stderr, format, args);
    fflush(stderr);
    va_end(args);
    abort();
}

void* mem_malloc(unsigned long n, char* file, int line)
{
    // fprintf(stdout, "Allocation at %s, line %d\n", file, line);
    if(n == 0) return 0;

    void* ptr = malloc(n);
    unsigned long uln = n;
    if(!ptr && n > 0)
    {
        mem_error("Memory allocation of %lu bytes failed", uln);
    }
    bytes += uln;
	used_bytes += uln;
    allocs++;
    return ptr;
}

void* mem_calloc(unsigned long n0, unsigned long n1)
{
    void* ptr = calloc(n0, n1);
    if(!ptr && n0 > 0 && n1 > 0)
    {
        mem_error("Memory allocation of %lu bytes failed", n0 * n1);
    }
    bytes += n0 * n1;
    used_bytes += n0 * n1;
    allocs++;
    return ptr;
}

void* mem_realloc(void* ptr, unsigned long n)
{
    bytes -= _msize(ptr);

    void* ret = realloc(ptr, n);
    unsigned long uln = n;
    if(ret == 0 && n > 0)
    {
        mem_error("Rellocation of pointer %p to size %lu failed\n", ptr, uln);
    }
    bytes += uln;
    return ret;
}

void mem_free(void* ptr)
{
    if(ptr == 0) return;

    bytes -= _msize(ptr);
    frees++;
    free(ptr);
    ptr = 0;
}

void mem_leak_check()
{
    fprintf(stdout, "\nAllocations:\n");
    fprintf(stdout, "  %d allocations.\n", allocs);
    fprintf(stdout, "  %d deallocations.\n", frees);
    fprintf(stdout, "  %lu bytes leaked.\n", bytes);
    fprintf(stdout, "Byte usage:\n");
    fprintf(stdout, "  %lu bytes used.\n", used_bytes);
    fprintf(stdout, "  %lu kilo bytes used.\n", used_bytes / 1024);
    fprintf(stdout, "  %lu mega bytes used.\n", used_bytes / 1024 / 1024);
}

#endif

#ifndef mem_h
#define mem_h

#define malloc(x) mem_malloc(x, __FILE__, __LINE__)
#define realloc(x, y) mem_realloc(x, y)
#define calloc(x, y) mem_calloc(x, y);
#define free(x) do { if ((x) != 0) {mem_free(x); (x)=0;} } while(0)

extern void* mem_malloc(unsigned long n, char* file, int line);
extern void* mem_realloc(void* ptr, unsigned long n);
extern void* mem_calloc(unsigned long n0, unsigned long n1);
extern void mem_free(void* ptr);
extern void mem_leak_check();
extern void mem_error(const char* format, ...);

#endif
#else
#include <stdlib.h>
#endif
