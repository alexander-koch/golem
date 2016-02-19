#ifndef stack_h
#define stack_h

#include <core/mem.h>

typedef struct stack_t {
	void** content;
	int size;
	int top;
} stack_t;

stack_t* stack_new();
void stack_resize(stack_t* s, int size);
void stack_push(stack_t* s, void* val);
void* stack_top(stack_t* s);
void* stack_pop(stack_t* s);
int stack_size(stack_t* s);
int stack_empty(stack_t* s);
void stack_free(stack_t* s);

#endif
