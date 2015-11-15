#ifndef util_h
#define util_h

#include <core/mem.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

// String operations
char* strdup(const char* str);
char* strndup(const char* str, size_t n);
char* concat(char *s1, char *s2);

// Custom format function uses $ for elements
char* strf(const char* fmt, ...);

// djb2 hashing algorithm
unsigned long djb2(unsigned char* str);

// File reading methods
char* readFile(const char* filename, size_t* len);
char* replaceExt(char* filename, const char* ext, size_t len);

// Memory
void memset64(void* dest, uint64_t value, uintptr_t size);

// Mersenne-Twister
void seed_prng(const uint32_t seed_value);
double prng();

#endif
