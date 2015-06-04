#include "value.h"

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
	return val;
}

value_t* value_new_int(long number)
{
	value_t* val = value_new_null();
	val->type = VALUE_INT;
	val->v.i = number;
	return val;
}

value_t* value_new_float(double number)
{
	value_t* val = value_new_null();
	val->type = VALUE_FLOAT;
	val->v.f = number;
	return val;
}

value_t* value_new_string(const char* string)
{
	value_t* val = value_new_null();
	val->type = VALUE_STRING;
	val->v.str = strdup(string);
	return val;
}

value_t* value_new_object(void* obj)
{
	value_t* val = value_new_null();
	val->type = VALUE_OBJECT;
	val->v.o = obj;
	return val;
}

void value_free(value_t* value)
{
	if(value)
	{
		if(value->type == VALUE_STRING)
		{
			free(value->v.str);
		}
		else if(value->type == VALUE_OBJECT)
		{
			free(value->v.o);
		}
		free(value);
	}
}

void value_print(value_t* value)
{
	if(value)
	{
		switch(value->type)
		{
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
			default: break;
		}
	}
}
