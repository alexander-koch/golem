/**
 * vector.h
 * @author Alexander Koch
 * @desc Vector implementation
 */

#ifndef vector_h
#define vector_h

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <core/mem.h>

#define VECTOR_CAPACITY 15

typedef struct vector_t {
	void** data;
	size_t capacity;
	size_t size;
} vector_t;

vector_t* vector_new();
void vector_push(vector_t* vector, void* data);
void* vector_pop(vector_t* vector);
void* vector_top(vector_t* vector);
void vector_set(vector_t* vector, size_t index, void* data);
void* vector_get(vector_t* vector, size_t index);
size_t vector_size(const vector_t* vector);
void vector_free(vector_t* vector);

#endif
