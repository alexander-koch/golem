#ifndef api_h
#define api_h

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <core/mem.h>

typedef int8_t I8;
typedef uint8_t U8;
typedef int16_t I16;
typedef uint16_t U16;
typedef int32_t I32;
typedef uint32_t U32;
typedef int64_t I64;
typedef uint64_t U64;
typedef float F32;
typedef double F64;

void console(const char* format, ...);

#endif
