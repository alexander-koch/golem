#include "util.h"

char* strdup(const char* str)
{
    size_t len = strlen(str) + 1;
	char *copy = malloc(len);
	if(!copy) return 0;
	memcpy(copy, str, len);
	return copy;
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

char* strf(const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    size_t totalLength = 0;
    for (const char* c = format; *c != '\0'; c++)
    {
        switch (*c)
        {
            case '$':
            totalLength += strlen(va_arg(argList, const char*));
            break;
            default:
            totalLength++;
        }
    }
    va_end(argList);

    char* buffer = malloc(sizeof(char)*totalLength);
    va_start(argList, format);
    char* start = buffer;
    for(const char* c = format; *c != '\0'; c++)
    {
        switch (*c)
        {
            case '$':
            {
                const char* string = va_arg(argList, const char*);
                size_t length = strlen(string);
                memcpy(start, string, length);
                start += length;
                break;
            }
            default:
            *start++ = *c;
        }
    }
    va_end(argList);

    return buffer;
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

char* readFile(const char* filename, size_t* len)
{
    *len = 0;
    FILE* file = fopen(filename, "rb");
    if(!file) return 0;
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);
    char* source = (char*)malloc(size+1);
    fread(source, sizeof(char), size, file);
    source[size] = '\0';
    fclose(file);
    *len = size;
    return source;
}

char* replaceExt(char* filename, const char* ext, size_t len)
{
    char* x = strrchr(filename, '.');
    size_t idx = x - filename;
    char* mem = malloc(sizeof(char) * (idx+len+1));
    memset(mem, 0, idx+len+1);
    memcpy(mem, x-idx, idx);
    memcpy(mem+idx, ext, len);
    idx += len+1;
    mem[idx] = '\0';
    return mem;
}

void memset64(void* dest, uint64_t value, uintptr_t size)
{
    uintptr_t i;
    for(i = 0; i < (size & (~7)); i+=8)
    {
        memcpy(((char*)dest) + i, &value, 8);
    }
    for(; i < size; i++)
    {
        ((char*)dest)[i] = ((char*)&value)[i&7];
    }
}
