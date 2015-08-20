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

value_t* value_new_array(value_t** data, size_t length)
{
	array_t* array = malloc(sizeof(*array));
	array->data = data;
	array->size = length;

	value_t* val = value_new_null();
	val->type = VALUE_ARRAY;
	val->v.o = array;
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
	else if(val->type == VALUE_ARRAY)
	{
		array_t* arr = value_array(value);
		array_t* newArr = malloc(sizeof(*newArr));
		newArr->data = malloc(sizeof(value_t*) * arr->size);
		newArr->size = arr->size;
		for(int i = 0; i < arr->size; i++)
		{
			newArr->data[i] = value_copy(arr->data[i]);
		}
		val->v.o = newArr;
	}
	else
	{
		val->v = value->v;
	}
	return val;
}

void value_reset(value_t* value)
{
	if(value->type == VALUE_STRING)
	{
		free(value->v.o);
		value->v.o = 0;
	}
	else if(value->type == VALUE_ARRAY)
	{
		array_t* arr = value_array(value);
		for(int i = 0; i < arr->size; i++)
		{
			value_free(arr->data[i]);
		}
		free(arr->data);
		free(arr);
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
			case VALUE_ARRAY:
			{
				array_t* arr = value_array(value);
				console("[");
				for(int i = 0; i < arr->size; i++)
				{
					value_t* idx = arr->data[i];
					value_print(idx);
					if(i < arr->size-1) console(", ");
				}
				console("]");

				// console("[");
				// list_t* list = (list_t*)value->v.o;
				// list_iterator_t* iter = list_iterator_create(list);
				// while(!list_iterator_end(iter))
				// {
				// 	value_t* val = list_iterator_next(iter);
				// 	value_print(val);
				// 	if(!list_iterator_end(iter)) console(", ");
				// }
				// list_iterator_free(iter);
				// console("]");
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
