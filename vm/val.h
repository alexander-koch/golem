/**
 * val.h
 * Copyright (C) 2017 Alexander Koch
 * Generic value definition
 *
 * NaN-Tagging to store values on IEEE 754 doubles.
 */

#ifndef val_h
#define val_h

#include <stdint.h>
#include <stdbool.h>
#include <core/mem.h>
#include <core/util.h>

// NaN (Not a Number) Tagging Introduction:
// ------------------------
//
// IEEE 754 double-precision float: 64-bit (8-byte) value with the following layout:
//
// 1 Sign bit
// | 11 Exponent bits
// | |          52 Mantissa (i.e. fraction) bits
// | |          |
// S[Exponent-][Mantissa------------------------------------------]
//
// A NaN is any value where all exponent bits are set:
//
//  v-- NaN bits
// -11111111111----------------------------------------------------
// S[Exponent-][Mantissa------------------------------------------]
//
// If all of the NaN bits are set, it's not a number. Otherwise, it is.
// Also we need quiet NaNs, so the first Mantissa bit has to be set as well.
//
//               v-- first Mantissa bit
// -[NaN      ]1---------------------------------------------------
//
// To distinguish between "true", "false" or "null" the second three bits are used.
//
//                                                             vvv-- 3 bits
// -[NaN      ]1---------------------------------------------------
//
// Pointers can use the rest of the mantissa bits (51)

// Definition

typedef uint64_t val_t;

// Class subtype
typedef struct obj_class_t {
    val_t* fields;
    unsigned int field_count;
} obj_class_t;

typedef struct obj_array_t {
    val_t* data;
    size_t len;
} obj_array_t;

// Object types
typedef enum obj_type_t {
    OBJ_NULL,
    OBJ_STRING,
    OBJ_ARRAY,
    OBJ_CLASS
} obj_type_t;

// Object definition
typedef struct obj_t {
    obj_type_t type;
    void* data;
    unsigned char marked;
    struct obj_t* next;
} obj_t;

obj_t* obj_new();
obj_t* obj_string_const_new(const char* str);
obj_t* obj_string_new(char* str);
obj_t* obj_string_nocopy_new(char* str);
obj_t* obj_array_new(val_t* data, size_t length);
obj_t* obj_class_new(int fields);
void obj_free(obj_t* obj);

// Util

// The first Sign bit
#define SIGN_BIT ((uint64_t)1 << 63)

// All bits for a quiet Nan (11 Exponent) + (1 Mantissa)
#define QNAN ((uint64_t)0x7ffc000000000000)

#define TAG_NAN       (0)
#define TAG_NULL      (1)
#define TAG_FALSE     (2)
#define TAG_TRUE      (3)
#define TAG_UNDEFINED (4)
#define TAG_UNUSED2   (5)
#define TAG_UNUSED3   (6)
#define TAG_UNUSED4   (7)

#define NULL_VAL      ((val_t)(uint64_t)(QNAN | TAG_NULL))
#define FALSE_VAL     ((val_t)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL      ((val_t)(uint64_t)(QNAN | TAG_TRUE))
#define UNDEFINED_VAL ((val_t)(uint64_t)(QNAN | TAG_UNDEFINED))

// Testing

// If quiet nan is not set, it is a number
#define IS_NUM(value) (((value) & QNAN) != QNAN)
#define IS_BOOL(value) ((value) == TRUE_VAL || (value) == FALSE_VAL)
#define IS_INT32(value) (val_is_int32(value))
// If the value is a pointer, the nan and the sign is set
#define IS_OBJ(value) (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define IS_STRING(value) (IS_OBJ(value) && ((obj_t*)AS_OBJ(value))->type == OBJ_STRING)
#define IS_ARRAY(value) (IS_OBJ(value) && ((obj_t*)AS_OBJ(value))->type == OBJ_ARRAY)
#define IS_CLASS(value) (IS_OBJ(value) && ((obj_t*)AS_OBJ(value))->type == OBJ_CLASS)

// Interpreting

#define AS_BOOL(value) ((value) == TRUE_VAL)
#define AS_NUM(value) (val_to_double(value))
#define AS_INT32(value) (val_to_int32(value))
#define AS_OBJ(value) ((obj_t*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))
#define AS_STRING(value) ((char*)(((obj_t*)AS_OBJ(value))->data))
#define AS_ARRAY(value) ((obj_array_t*)(((obj_t*)AS_OBJ(value))->data))
#define AS_CLASS(value) ((obj_class_t*)(((obj_t*)AS_OBJ(value))->data))

// Converting

#define BOOL_VAL(b) (val_t)(b ? TRUE_VAL : FALSE_VAL)
#define NUM_VAL(num) (val_of_double(num))
#define INT32_VAL(num) (val_of_int32(num))
#define OBJ_VAL(obj) (val_t)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))
#define STRING_CONST_VAL(p) (val_t)(OBJ_VAL(obj_string_const_new(p)))
#define STRING_VAL(p) (val_t)(OBJ_VAL(obj_string_new(p)))
#define STRING_NOCOPY_VAL(p) (val_t)(OBJ_VAL(obj_string_nocopy_new(p)))

#define COPY_VAL(p) (val_copy(p))
#define COPY_OBJ(p) (obj_copy(p))

bool val_is_int32(val_t val);
val_t val_of_int32(int32_t i);
int val_to_int32(val_t val);

obj_t* obj_copy(obj_t* obj);

double val_to_double(val_t value);
val_t val_of_double(double num);
bool val_equal(val_t v1, val_t v2);
val_t val_copy(val_t val);
void val_free(val_t v1);

char* val_tostr(val_t v1);
void val_print(val_t v1);

#endif
