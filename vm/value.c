#include "value.h"

value_t* value_new_null()
{
	value_t* val = malloc(sizeof(*val));
	val->type = VALUE_NULL;
	val->marked = 0;
	val->next = 0;
	val->v.o = 0;
	return val;
}

value_t* value_new_bool(bool b)
{
	value_t* val = value_new_null();
	val->type = VALUE_BOOL;
	val->v.i = b;
	return val;
}

value_t* value_new_int(int number)
{
	value_t* val = value_new_null();
	val->type = VALUE_INT;
	val->v.i = number;
	return val;
}

value_t* value_new_float(float number)
{
	value_t* val = value_new_null();
	val->type = VALUE_FLOAT;
	val->v.f = number;
	return val;
}

value_t* value_new_char(char character)
{
	value_t* val = value_new_null();
	val->type = VALUE_CHAR;
	val->v.c = character;
	return val;
}

value_t* value_new_string_const(const char* string)
{
	value_t* val = value_new_null();
	val->type = VALUE_STRING;
	val->v.o = strdup(string);
	return val;
}

value_t* value_new_string(char* string)
{
	value_t* val = value_new_null();
	val->type = VALUE_STRING;
	val->v.o = strdup(string);
	return val;
}

value_t* value_new_string_nocopy(char* string)
{
	value_t* val = value_new_null();
	val->type = VALUE_STRING;
	val->v.o = string;
	return val;
}

value_t* value_new_list(list_t* list)
{
	value_t* val = value_new_null();
	val->type = VALUE_LIST;
	val->v.o = list;
	return val;
}

value_t* value_new_object(void* obj)
{
	value_t* val = value_new_null();
	val->type = VALUE_OBJECT;
	val->v.o = obj;
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
	else if(val->type == VALUE_LIST)
	{
		// TODO: computationally expensive, improve the performance

		list_t* list = list_new();
		list_iterator_t* iter = list_iterator_create(value->v.o);
		while(!list_iterator_end(iter))
		{
			value_t* val = list_iterator_next(iter);
			list_push(list, value_copy(val));
		}
		list_iterator_free(iter);
		val->v.o = list;
	}
	else
	{
		val->v = value->v;
	}

	//TODO: obj pointer copying
	//FIXME: unstable could possibly create an error, doesn't work for objects

	return val;
}

void value_reset(value_t* value)
{
	if(value->type == VALUE_STRING)
	{
		free(value->v.o);
		value->v.o = 0;
	}
	else if(value->type == VALUE_LIST)
	{
		list_t* list = (list_t*)value->v.o;
		list_iterator_t* iter = list_iterator_create(list);
		while(!list_iterator_end(iter))
		{
			value_t* val = list_iterator_next(iter);
			value_free(val);
		}
		list_iterator_free(iter);
		list_free(list);
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
			case VALUE_CHAR:
			{
				console("%c", value->v.c);
				break;
			}
			case VALUE_LIST:
			{
				console("[");
				list_t* list = (list_t*)value->v.o;
				list_iterator_t* iter = list_iterator_create(list);
				while(!list_iterator_end(iter))
				{
					value_t* val = list_iterator_next(iter);
					value_print(val);
					if(!list_iterator_end(iter)) console(", ");
				}
				list_iterator_free(iter);
				console("]");
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
