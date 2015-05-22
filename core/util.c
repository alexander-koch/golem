#include "util.h"

char* strdup(const char* str)
{
    const char* t = str;
    size_t n = strlen(str);
    char* r = malloc(n + 1);
    char* p = r;
    while(*t && t < str + n)
    {
        *p++ = *t++;
    }
    *p = 0;
    return r;
}

char* strndup(const char* str, size_t n)
{
    const char* t = str;
    char* r = malloc(n + 1);
    char* p = r;
    while(*t && t < str + n)
    {
        *p++ = *t++;
    }
    *p = 0;
    return r;
}
