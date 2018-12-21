/**
 * mem.h
 * Copyright (C) 2017 Alexander Koch
 * Memory debug helper
 */

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

#ifdef __MINGW32__
    #define ptr_size _msize
#else
    #define ptr_size malloc_usable_size
#endif

void mem_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Fatal error: ");
    vfprintf(stderr, format, args);
    fflush(stderr);
    va_end(args);
    abort();
}

void* mem_malloc(unsigned long n, char* file, int line) {
    // fprintf(stdout, "Allocation at %s, line %d\n", file, line);
    if(n == 0) return 0;

    void* ptr = malloc(n);
    unsigned long uln = ptr_size(ptr);
    if(!ptr && n > 0) {
        mem_error("Memory allocation of %lu bytes failed", uln);
    }
    bytes += uln;
	used_bytes += uln;
    allocs++;
    return ptr;
}

void* mem_calloc(unsigned long n0, unsigned long n1) {
    void* ptr = calloc(n0, n1);
    unsigned long uln = ptr_size(ptr);
    if(!ptr && n0 > 0 && n1 > 0) {
        mem_error("Memory allocation of %lu bytes failed", uln);
    }
    bytes += uln;
    used_bytes += uln;
    allocs++;
    return ptr;
}

void* mem_realloc(void* ptr, unsigned long n) {
    bytes -= ptr_size(ptr);

    void* ret = realloc(ptr, n);
    unsigned long uln = ptr_size(ret);
    if(ret == 0 && n > 0) {
        mem_error("Rellocation of pointer %p to size %lu failed\n", ptr, uln);
    }
    bytes += uln;
    return ret;
}

void mem_free(void* ptr) {
    if(ptr == 0) return;

    bytes -= ptr_size(ptr);
    frees++;
    free(ptr);
    ptr = 0;
}

void mem_leak_check(void) {
    printf("\nAllocations:\n");
    printf("  %d allocations.\n", allocs);
    printf("  %d deallocations.\n", frees);
    printf("  %lu bytes leaked.\n", bytes);
    printf("Byte usage:\n");
    printf("  %lu bytes used.\n", used_bytes);
    printf("  %lu kilo bytes used.\n", used_bytes / 1024);
    printf("  %lu mega bytes used.\n", used_bytes / 1024 / 1024);
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
