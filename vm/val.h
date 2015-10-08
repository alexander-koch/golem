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
//			   v-- first Mantissa bit
// -[NaN      ]1---------------------------------------------------
//
// To distinguish between "true", "false" or "null" the first three bits are used
//
//																vvv-- 3 bits
// -[NaN      ]1---------------------------------------------------
//
// Pointers can use the rest of the mantissa bits (51)

// Definition

typedef uint64_t val_t;

typedef enum obj_type_t
{
	OBJ_NULL,
	OBJ_STRING,
	OBJ_CLASS
} obj_type_t;

typedef struct obj_class_t
{
	val_t fields[128];
	size_t fp;
} obj_class_t;

typedef struct obj_array_t
{
	void** data;
	size_t sz;
} obj_array_t;

typedef struct obj_t
{
	obj_type_t type;
	void* data;
	unsigned char marked;
	struct obj_t* next;
} obj_t;

obj_t* obj_new();
obj_t* obj_string_const_new(const char* str);
obj_t* obj_string_new(char* str);
void obj_free(obj_t* obj);

#define AS_STRING(obj) (char*)(obj->data)

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
// If the value is a pointer the nan and the sign is set
#define IS_OBJ(value) (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

// Interpreting

#define AS_BOOL(value) ((value) == TRUE_VAL)
#define AS_NUM(value) (val_to_double(value))
#define AS_OBJ(value) ((obj_t*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

// Converting

#define BOOL_VAL(b) (val_t)(b ? TRUE_VAL : FALSE_VAL)
#define NUM_VAL(num) (val_t)(val_of_double(num))
#define OBJ_VAL(obj) (val_t)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

double val_to_double(val_t value);
val_t val_of_double(double num);
bool val_equal(val_t v1, val_t v2);
void val_free(val_t v1);
void val_print(val_t v1);

#endif
