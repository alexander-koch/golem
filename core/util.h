#ifndef util_h
#define util_h

#include <core/mem.h>
#include <string.h>

char* strdup(const char* str);
char* strndup(const char* str, size_t n);
char* concat(char *s1, char *s2);

#endif
