#ifndef value_h
#define value_h

#include <stdbool.h>
#include <core/mem.h>
#include <core/api.h>
#include <core/util.h>
#include <adt/list.h>
#include <adt/vector.h>

#ifdef NAN_TAGGING
// IEEE 754 double-precision float: 64-bit value with the following layout:
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
// Also we need quiet NaNs, so the first Mantissa bit has to be set.
//
//			   v-- first Mantissa bit
// -[NaN      ]1---------------------------------------------------
//
// To distinguish between "true", "false" or "null" the first three bits are used
//
//																 v--- 3 bits
// -[NaN      ]1------------------------------------------------[I]
//
// Pointers can use the rest of the mantissa bits (51)
typedef uint64_t value_t;

// The first Sign bit
#define SIGN_BIT ((uint64_t)1 << 63)

// All 11 bit for a quiet Nan
#define QNAN ((uint64_t)0x7ffc000000000000)

#define TAG_NAN       (0)
#define TAG_NULL      (1)
#define TAG_FALSE     (2)
#define TAG_TRUE      (3)
#define TAG_UNDEFINED (4)
#define TAG_UNUSED2   (5)
#define TAG_UNUSED3   (6)
#define TAG_UNUSED4   (7)

#define NULL_VAL      ((value_t)(uint64_t)(QNAN | TAG_NULL))
#define FALSE_VAL     ((value_t)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL      ((value_t)(uint64_t)(QNAN | TAG_TRUE))
#define UNDEFINED_VAL ((value_t)(uint64_t)(QNAN | TAG_UNDEFINED))

// If quiet nan is not set, it is a number
#define IS_NUM(value) (((value) & QNAN) != QNAN)
#define IS_BOOL(value) ((value) == TRUE_VAL || (value) == FALSE_VAL)

// If the value is a pointer the nan and the sign is set
#define IS_OBJ(value) (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(value) ((value) == TRUE_VAL)
#define AS_PTR(value) ((obj_t*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

#define TO_PTR(obj) (value_t)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj));

typedef union
{
  uint64_t bits64;
  uint32_t bits32[2];
  double num;
} DoubleBits;

double value_to_num(value_t value)
{
	DoubleBits data;
	data.bits64 = value;
	return data.num;
}

value_t value_of_num(double num)
{
	DoubleBits data;
	data.num = num;
	return data.bits64;
}

bool value_equal(value_t v1, value_t v2)
{
	return v1 == v2;
}

#endif

// Main stuff

#define value_set_bool(val, b) val->v.i = b
#define value_set_int(val, in) val->v.i = in
#define value_set_float(val, fl) val->v.f = fl
#define value_set_char(val, ch) val->v.c = ch

#define value_bool(val) (bool)val->v.i
#define value_int(val) val->v.i
#define value_float(val) val->v.f
#define value_number(val) (val->type == VALUE_INT ? value_int(val) : value_float(val))
#define value_char(val) val->v.c
#define value_string(val) (char*)val->v.o
#define value_array(val) (array_t*)val->v.o
#define value_class(val) (class_t*)val->v.o

typedef enum
{
	VALUE_NULL,
	VALUE_BOOL,
	VALUE_FLOAT,
	VALUE_INT,
	VALUE_CHAR,
	VALUE_STRING,
	VALUE_ARRAY,
	VALUE_CLASS,
} value_type_t;

typedef struct value_t
{
	value_type_t type;
	union
	{
		char c;
		int i;
	 	float f;
		void* o;
	} v;

	unsigned char marked;
	struct value_t* next;
} value_t;

typedef struct array_t
{
	value_t** data;
	size_t size;
} array_t;

typedef struct class_t
{
	vector_t* fields;
	// int (*copy)(void*);
	// int (*compare)(void*);
	// int (*destruct)(void*);
} class_t;

// Standard values
value_t* value_new_null();
value_t* value_new_bool(bool b);
value_t* value_new_int(int number);
value_t* value_new_float(float number);
value_t* value_new_char(char character);
value_t* value_new_string_const(const char* string);
value_t* value_new_string(char* string);
value_t* value_new_string_nocopy(char* string);
value_t* value_new_array(value_t** data, size_t length);
value_t* value_new_array_nocopy(array_t* data);
value_t* value_new_class();

// Value copy
value_t* value_copy(value_t* value);
bool value_equals(value_t* v1, value_t* v2);

// Value operations
void value_reset(value_t* value);
void value_free(value_t* value);
void value_print(value_t* value);

#endif
