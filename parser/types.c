// Copyright (C) 2017 Alexander Koch
#include "types.h"

datatype_t* datatype_new(type_t base) {
    datatype_t* t = malloc(sizeof(datatype_t));
    t->type = base;
    t->id = 0;
    t->subtype = 0;
    return t;
}

datatype_t* datatype_copy(datatype_t* other) {
    datatype_t* t = malloc(sizeof(datatype_t));
    t->type = other->type;
    t->id = other->id;
    if(other->subtype) {
        t->subtype = datatype_copy(other->subtype);
    } else {
        t->subtype = 0;
    }
    return t;
}

bool datatype_match(datatype_t* t1, datatype_t* t2) {
    bool cond = t1->type == t2->type && t1->id == t2->id;
    if(t1->subtype) {
        if(!t2->subtype) {
            return false;
        } else {
            return datatype_match(t1->subtype, t2->subtype) && cond;
        }
    }
    if(t2->subtype) {
        // T1 does not have a subtype
        return false;
    }
    return cond;
}

void datatype_free(datatype_t* dt) {
    if(dt->subtype) datatype_free(dt->subtype);
    free(dt);
}

const char* datatype_str(datatype_t* t) {
    switch(t->type) {
        case DATA_NULL: return "null";
        case DATA_BOOL: return "bool";
        case DATA_INT: return "int";
        case DATA_FLOAT: return "float";
        case DATA_CHAR: return "char";
        case DATA_CLASS: return "type";
        case DATA_VOID: return "void";
        case DATA_GENERIC: return "generic";
        case DATA_OPTION: return "option";
        case DATA_ARRAY: {
            if(t->subtype) {
                switch(t->subtype->type) {
                    case DATA_CHAR: return "char[]";
                    case DATA_INT: return "int[]";
                    case DATA_BOOL: return "bool[]";
                    case DATA_FLOAT: return "float[]";
                    case DATA_CLASS: return "class[]";
                    case DATA_VOID: return "void[]";
                    case DATA_GENERIC: return "generic[]";
                    case DATA_OPTION: return "option[]";
                    default: return "null[]";
                }
            } else {
                return "undefined[]";
            }
        }
        default: return "undefined";
    }
}

context_t* context_new() {
    context_t* context = malloc(sizeof(context_t));
    context->types = hashmap_new();
    context->extra = vector_new();

    // Define null type
    context->null_type.type = DATA_NULL;
    context->null_type.id = 0;
    context->null_type.subtype = 0;

    // Define void type
    context->void_type = datatype_new(DATA_VOID);
    context_insert(context, "void", context->void_type);

    context_insert(context, "bool", datatype_new(DATA_BOOL));
    context_insert(context, "int", datatype_new(DATA_INT));
    context_insert(context, "float", datatype_new(DATA_FLOAT));
    context_insert(context, "char", datatype_new(DATA_CHAR));
    context_insert(context, "generic", datatype_new(DATA_GENERIC));
    context_insert(context, "option", datatype_new(DATA_OPTION));

    datatype_t* str_type = datatype_new(DATA_ARRAY);
    str_type->subtype = datatype_new(DATA_CHAR);
    context_insert(context, "str", str_type);
    return context;
}

void context_insert(context_t* context, char* name, datatype_t* type) {
    hashmap_set(context->types, name, type);
}

datatype_t* context_find_or_create(context_t* context, datatype_t* type) {
    for(unsigned i = 0; i < vector_size(context->extra); i++) {
        datatype_t* idx = vector_get(context->extra, i);
        if(datatype_match(idx, type)) {
            return idx;
        }
    }

    datatype_t* cp = datatype_copy(type);
    vector_push(context->extra, cp);
    return cp;
}

datatype_t* context_get(context_t* context, char* name) {
    return (datatype_t*)hashmap_find(context->types, name);
}

datatype_t* context_null(context_t* context) {
    return &context->null_type;
}

datatype_t* context_void(context_t* context) {
    return context->void_type;
}

int free_type(void* val, void* arg) {
    datatype_free((datatype_t*)val);
    return 0;
}

void context_free(context_t* context) {
    hashmap_foreach(context->types, free_type, 0);
    hashmap_free(context->types);

    for(unsigned i = 0; i < vector_size(context->extra); i++) {
        datatype_free(vector_get(context->extra, i));
    }

    vector_free(context->extra);
    free(context);
}
