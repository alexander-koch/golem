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

value_t* value_new_class()
{
	value_t* val = value_new_null();
	val->type = VALUE_CLASS;

	class_t* clazz = malloc(sizeof(*clazz));
	clazz->fields = vector_new();
	val->v.o = clazz;
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
		// Create new array
		array_t* arr 	= value_array(value);
		array_t* newArr = malloc(sizeof(*newArr));
		newArr->data	= malloc(sizeof(value_t*) * arr->size);
		newArr->size 	= arr->size;

		// Fill array
		for(int i = 0; i < arr->size; i++)
		{
			newArr->data[i] = value_copy(arr->data[i]);
		}

		// Save in void*
		val->v.o = newArr;
	}
	else if(val->type == VALUE_CLASS)
	{
		// Create a copy
		class_t* original = value_class(value);
		class_t* instance = malloc(sizeof(*instance));
		instance->fields = vector_new();

		// Fill new vector
		for(int i = 0; i < vector_size(original->fields); i++)
		{
			value_t* idx = vector_get(original->fields, i);
			vector_push(instance->fields, value_copy(idx));
		}

		// Set to instance
		val->v.o = instance;
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
	}
	else if(value->type == VALUE_CLASS)
	{
		class_t* clazz = value_class(value);
		for(int i = 0; i < vector_size(clazz->fields); i++)
		{
			value_free(vector_get(clazz->fields, i));
		}
		vector_free(clazz->fields);
		free(clazz);
	}
	value->v.o = 0;
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
				break;
			}
			case VALUE_CLASS:
			{
				class_t* cls = value_class(value);
				console("class: <");
				for(int i = 0; i < vector_size(cls->fields); i++)
				{
					value_t* v = vector_get(cls->fields, i);
					value_print(v);
					if(i < vector_size(cls->fields)-1) console(", ");
				}
				console(">");
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
