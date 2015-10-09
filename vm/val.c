#include "val.h"

typedef union
{
  uint64_t bits64;
  uint32_t bits32[2];
  double num;
} doublebits_t;

double val_to_double(val_t value)
{
	doublebits_t data;
	data.bits64 = value;
	return data.num;
}

val_t val_of_double(double num)
{
	doublebits_t data;
	data.num = num;
	return data.bits64;
}

bool val_equal(val_t v1, val_t v2)
{
	return v1 == v2;
}

obj_t* obj_new()
{
	obj_t* obj = malloc(sizeof(*obj));
	obj->type = OBJ_NULL;
	obj->data = 0;
	obj->marked = 0;
	obj->next = 0;
	return obj;
}

obj_t* obj_string_const_new(const char* str)
{
	obj_t* obj = obj_new();
	obj->type = OBJ_STRING;
	obj->data = strdup(str);
	return obj;
}

obj_t* obj_string_new(char* str)
{
	obj_t* obj = obj_new();
	obj->type = OBJ_STRING;
	obj->data = strdup(str);
	return obj;
}

obj_t* obj_string_nocopy_new(char* str)
{
    obj_t* obj = obj_new();
    obj->type = OBJ_STRING;
    obj->data = str;
    return obj;
}

obj_t* obj_array_new(val_t* data, size_t length)
{
    obj_t* obj = obj_new();
    obj->type = OBJ_ARRAY;
    obj->data = data;
    return obj;
}

obj_t* obj_class_new()
{
    obj_t* obj = obj_new();
    obj->type = OBJ_CLASS;

    obj_class_t* cls = malloc(sizeof(*cls));
    val_t nil = NULL_VAL;
    memset(cls->fields, nil, sizeof(val_t) * CLASS_FIELDS_SIZE);

    obj->data = cls;
    return obj;
}

void obj_free(obj_t* obj)
{
    switch(obj->type)
    {
        case OBJ_STRING:
        case OBJ_ARRAY:
        case OBJ_CLASS:
        {
            free(obj->data);
            break;
        }
        default: break;
    }
    free(obj);
}

void val_free(val_t v1)
{
	if(IS_OBJ(v1))
	{
		obj_t* obj = AS_OBJ(v1);
		obj_free(obj);
	}
}

void val_print(val_t v1)
{
    if(IS_NUM(v1))
    {
        printf("%f", AS_NUM(v1));
    }
    else if(IS_BOOL(v1))
    {
        printf("%s", AS_BOOL(v1) ? "true" : "false");
    }
    else if(IS_OBJ(v1))
    {
        obj_t* obj = AS_OBJ(v1);
        switch(obj->type)
        {
            case OBJ_STRING:
            {
                printf("%s", (char*)obj->data);
                break;
            }
            case OBJ_ARRAY:
            {
                printf("array<%x>", (unsigned int)v1);
                break;
            }
            case OBJ_CLASS:
            {
                printf("class<%x>", (unsigned int)v1);
                break;
            }
            default: break;
        }
    }
    else
    {
        printf("NULL");
    }
}
