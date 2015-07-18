#include "value.h"

bool value_bool(value_t* value)
{
	return value->v.b;
}

I64 value_int(value_t* value)
{
	return value->v.i;
}

F64 value_float(value_t* value)
{
	return value->v.f;
}

char* value_string(value_t* value)
{
	return value->v.str;
}

value_t* value_new_null()
{
	value_t* val = malloc(sizeof(*val));
	val->type = VALUE_NULL;
	val->refcount = 0;
	return val;
}

value_t* value_new_bool(bool b)
{
	value_t* val = value_new_null();
	val->type = VALUE_BOOL;
	val->v.b = b;
	val->classname = "bool";
	return val;
}

value_t* value_new_int(long number)
{
	value_t* val = value_new_null();
	val->type = VALUE_INT;
	val->v.i = number;
	val->classname = "int";
	return val;
}

value_t* value_new_float(double number)
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
	val->v.str = strdup(string);
	val->classname = "string";
	return val;
}

value_t* value_new_string(char* string)
{
	value_t* val = value_new_null();
	val->type = VALUE_STRING;
	val->v.str = strdup(string);
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
		val->v.str = strdup(value->v.str);
	}
	else
	{
		val->v = value->v;
	}
	val->classname = value->classname;

	//TODO: obj pointer copying
	//FIX UNSTABLE: could possibly create an error

	return val;
}

const char* value_classname(value_t* value)
{
	return value->classname;
}

void value_retain(value_t* value)
{
	if(value)
	{
		value->refcount++;
	}
}

void value_reset(value_t* value)
{
	if(value->type == VALUE_STRING)
	{
		free(value->v.str);
		value->v.str = 0;
	}
	else if(value->type == VALUE_OBJECT)
	{
		free(value->v.o);
		value->v.o = 0;
	}
}

void value_free(value_t* value)
{
	if(!value) return;

	if(value->refcount == 0)
	{
		value_reset(value);
		free(value);
	}
	else
	{
		value->refcount--;
	}
}

void value_print(value_t* value)
{
	if(value)
	{
		switch(value->type)
		{
			case VALUE_BOOL:
			{
				console("%s", value->v.b ? "true" : "false");
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
				console("%s", value->v.str);
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
