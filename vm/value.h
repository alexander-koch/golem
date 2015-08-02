#ifndef value_h
#define value_h

// Precision loss?
// TODO: improve

#include <stdbool.h>
#include <core/mem.h>
#include <core/api.h>
#include <core/util.h>

#define value_set_bool(val, b) val->v.i = b
#define value_set_int(val, in) val->v.i = in
#define value_set_float(val, fl) val->v.f = fl

#define value_bool(val) (bool)val->v.i
#define value_int(val) val->v.i
#define value_float(val) val->v.f
#define value_string(val) (char*)val->v.o

#define value_number(v) (v->type == VALUE_INT ? value_int(v) : value_float(v))

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
	value_type_t type;
	union
	{
		int i;
	 	float f;

		struct {
			void* o;
			int (*func)(void*);
		};
	} v;

	unsigned char marked;
	struct value_t* next;
} value_t;

// Standard values
value_t* value_new_null();
value_t* value_new_bool(bool b);
value_t* value_new_int(int number);
value_t* value_new_float(float number);
value_t* value_new_string_const(const char* string);
value_t* value_new_string(char* string);
value_t* value_new_object(void* obj, int (*func)(void*));

// Value copy
value_t* value_copy(value_t* value);

// Value operations
void value_reset(value_t* value);
void value_free(value_t* value);
void value_print(value_t* value);

#endif
