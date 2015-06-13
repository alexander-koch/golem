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
	VALUE_OBJECT,
} value_type_t;

typedef struct value_t
{
	value_type_t type;
	size_t refcount;
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
value_t* value_new_bool(bool b);
value_t* value_new_int(long number);
value_t* value_new_float(double number);
value_t* value_new_string(const char* string);
value_t* value_new_object(void* obj);

void value_retain(value_t* value);
void value_reset(value_t* value);
void value_free(value_t* value);
void value_print(value_t* value);

bool value_bool(value_t* value);
I64 value_int(value_t* value);
F64 value_float(value_t* value);
char* value_string(value_t* value);

#endif
