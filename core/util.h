/**
 * util.h
 * @author Alexander Koch 2016
 * @desc Utility / helper functions.
 */

#ifndef util_h
#define util_h

#include <core/mem.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

// String operations
#if defined(__GNUC__) && !defined(__llvm__)
    #include <string.h>
    char* strdup(const char* str);
    char* strndup(const char* str, size_t n);
#else
    #define _POSIX_C_SOURCE 200809L
    #include <string.h>
#endif

char* concat(char *s1, char *s2);

// Custom format function uses $ for elements
char* strf(const char* fmt, ...);

// djb2 hashing algorithm
unsigned long djb2(unsigned char* str);

// File reading methods
char* readFile(const char* path);
char* replaceExt(char* filename, const char* ext, size_t len);
char* getDirectory(const char* path);

// Memory
void memset64(void* dest, uint64_t value, uintptr_t size);

// Mersenne-Twister
void seed_prng(const uint32_t seed_value);
double prng();

#endif
