#ifndef value_h
#define value_h

#include <stdbool.h>
#include <core/mem.h>
#include <core/api.h>
#include <core/util.h>
#include <adt/list.h>
#include <adt/vector.h>

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
