#ifndef value_h
#define value_h

#include <stdbool.h>
#include <core/mem.h>
#include <core/api.h>
#include <core/util.h>

typedef enum value_type_t
{
	VALUE_NULL,
	VALUE_BOOL,
	VALUE_FLOAT,
	VALUE_INT,
	VALUE_STRING,
	VALUE_STRUCT
} value_type_t;

typedef struct value_t
{
	value_type_t type;
	union
	{
		bool b;
		I64 i;
		F64 f;
		char* str;
		void* o;
	} v;
} value_t;

value_t* value_new_null();
value_t* value_new_int(long number);
value_t* value_new_float(double number);
value_t* value_new_string(const char* string);
void value_free(value_t* value);
void value_print(value_t* value);

#endif
