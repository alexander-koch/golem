#ifndef value_h
#define value_h

#include <stdbool.h>
#include <core/mem.h>
#include <core/api.h>
#include <core/util.h>

// Precision loss?
// TODO: improve

#define value_set_bool(val, b) val->v.i = b
#define value_set_int(val, in) val->v.i = in
#define value_set_float(val, f) val->v.f = f

typedef enum
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
	const char* classname;
	value_type_t type;

	unsigned int marked;
	union
	{
		int i;
	 	float f;
		char* str;
		void* o;
	} v;
} value_t;

value_t* value_new_null();
value_t* value_new_bool(bool b);
value_t* value_new_int(int number);
value_t* value_new_float(float number);
value_t* value_new_string_const(const char* string);
value_t* value_new_string(char* string);
value_t* value_new_object(void* obj);

value_t* value_copy(value_t* value);
const char* value_classname(value_t* value);

void value_reset(value_t* value);
void value_free(value_t* value);
void value_print(value_t* value);

void value_mark(value_t* value);

bool value_bool(value_t* value);
int value_int(value_t* value);
float value_float(value_t* value);
char* value_string(value_t* value);

#endif
