#ifndef types_h
#define types_h

#include <stdio.h>
#include <stdbool.h>
#include <adt/hashmap.h>
#include <adt/vector.h>
#include <core/mem.h>

typedef enum {
    DATA_NULL,
    DATA_BOOL,
    DATA_INT,
    DATA_FLOAT,
    DATA_CHAR,
    DATA_CLASS,
    DATA_VOID,
    DATA_ARRAY,
    DATA_GENERIC,
} type_t;

typedef struct datatype_t {
    type_t type;
    unsigned long id;
    struct datatype_t* subtype;
} datatype_t;

datatype_t* datatype_new(type_t base);
datatype_t* datatype_copy(datatype_t* other);
bool datatype_match(datatype_t* t1, datatype_t* t2);
void datatype_free(datatype_t* dt);
const char* datatype_str(datatype_t* type);

typedef struct context_t {
    hashmap_t* types;
    vector_t* extra;
    datatype_t null_type;
    datatype_t* void_type;
} context_t;

context_t* context_new();
void context_insert(context_t* context, char* name, datatype_t* type);
datatype_t* context_find_or_create(context_t* context, datatype_t* type);
datatype_t* context_get(context_t* context, char* name);
datatype_t* context_null(context_t* context);
datatype_t* context_void(context_t* context);
void context_free(context_t* context);


#endif
