#include "value.h"

value_t* value_new_null()
{
	value_t* val = malloc(sizeof(*val));
	val->type = VALUE_NULL;
	val->marked = 0;
	return val;
}

value_t* value_new_bool(bool b)
{
	value_t* val = value_new_null();
	val->type = VALUE_BOOL;
	val->v.i = b;
	val->classname = "bool";
	return val;
}

value_t* value_new_int(int number)
{
	value_t* val = value_new_null();
	val->type = VALUE_INT;
	val->v.i = number;
	val->classname = "int";
	return val;
}

value_t* value_new_float(float number)
{
	value_t* val = value_new_null();
	val->type = VALUE_FLOAT;
	val->v.f = number;
	val->classname = "float";
	return val;
}

value_t* value_new_string_const(const char* string)
{
	value_t* val = value_new_null();
	val->type = VALUE_STRING;
	val->v.o = strdup(string);
	val->classname = "string";
	return val;
}

value_t* value_new_string(char* string)
{
	value_t* val = value_new_null();
	val->type = VALUE_STRING;
	val->v.o = strdup(string);
	val->classname = "string";
	return val;
}

value_t* value_new_object(void* obj)
{
	value_t* val = value_new_null();
	val->type = VALUE_OBJECT;
	val->v.o = obj;
	val->classname = "object";
	return val;
}

value_t* value_copy(value_t* value)
{
	value_t* val = value_new_null();
	val->type = value->type;
	if(val->type == VALUE_STRING)
	{
		val->v.o = strdup(value->v.o);
	}
	else
	{
		val->v = value->v;
	}
	val->classname = value->classname;

	//TODO: obj pointer copying
	//FIXME: unstable could possibly create an error, doesn't work for objects

	return val;
}

const char* value_classname(value_t* value)
{
	return value->classname;
}

void value_reset(value_t* value)
{
	if(value->type == VALUE_STRING || value->type == VALUE_OBJECT)
	{
		free(value->v.o);
		value->v.o = 0;
	}
}

void value_free(value_t* value)
{
	if(!value) return;

	value_reset(value);
	free(value);
	value = 0;
}

void value_print(value_t* value)
{
	if(value)
	{
		switch(value->type)
		{
			case VALUE_BOOL:
			{
				console("%s", value->v.i ? "true" : "false");
				break;
			}
			case VALUE_INT:
			{
				console("%d", value->v.i);
				break;
			}
			case VALUE_FLOAT:
			{
				console("%f", value->v.f);
				break;
			}
			case VALUE_STRING:
			{
				console("%s", (char*)value->v.o);
				break;
			}
			case VALUE_NULL:
			{
				console("null");
				break;
			}
			default: break;
		}
	}
}

void value_mark(value_t* value)
{
	if(!value->marked)
	{
		value->marked = 1;
	}
}
