#include "util.h"

//static char* rootDirectory = 0;

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

char* readFile(const char* path)
{
    FILE* file = fopen(path, "rb");
    if(!file) return 0;

    // Get the size of the file
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if(!buffer) {
        printf("Could not read file \"%s\".\n", path);
        fclose(file);
        return 0;
    }

    size_t bytes = fread(buffer, sizeof(char), fileSize, file);
    if(bytes < fileSize) {
        printf("Could not read file \"%s\".\n", path);
        fclose(file);
        return 0;
    }

    buffer[bytes] = '\0';
    fclose(file);
    return buffer;
}

char* getDirectory(const char* path) {
    char* root = 0;
    const char* lastSlash = strrchr(path, '/');
    if(lastSlash != 0) {
        root = (char*)malloc(lastSlash - path + 2);
        memcpy(root, path, lastSlash - path + 1);
        root[lastSlash - path + 1] = '\0';
    }
    return root;
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

// Mersenne-Twister
#define N 624
#define M 397
#define HI 0x80000000
#define LO 0x7fffffff

static uint32_t seed = 5489UL;
static const uint32_t A[2] = { 0, 0x9908b0df };
static uint32_t y[N];
static int index = N+1;

void seed_prng(const uint32_t seed_value)
{
    seed = seed_value;
}

uint32_t mt()
{
    uint32_t e;
    if(index > N) {
        int i;
        y[0] = seed;

        for(i = 1; i < N; i++) {
            y[i] = (1812433253UL * (y[i-1] ^ (y[i-1] >> 30)) + i);
        }
    }
    if(index >= N) {
        int i;
        uint32_t h;

        for (i=0; i<N-M; ++i) {
            h = (y[i] & HI) | (y[i+1] & LO);
            y[i] = y[i+M] ^ (h >> 1) ^ A[h & 1];
        }
        for ( ; i<N-1; ++i) {
            h = (y[i] & HI) | (y[i+1] & LO);
            y[i] = y[i+(M-N)] ^ (h >> 1) ^ A[h & 1];
        }

        h = (y[N-1] & HI) | (y[0] & LO);
        y[N-1] = y[M-1] ^ (h >> 1) ^ A[h & 1];
        index = 0;
    }

    e = y[index++];
    e ^= (e >> 11);
    e ^= (e <<  7) & 0x9d2c5680;
    e ^= (e << 15) & 0xefc60000;
    e ^= (e >> 18);

    return e;
}

double prng()
{
    uint32_t rnd = mt();
    double res = rnd / (double)UINT32_MAX;
    return res;
}
