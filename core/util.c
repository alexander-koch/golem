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

char* concat(char* s1, char* s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    if(!result) return 0;
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

unsigned long djb2(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}
