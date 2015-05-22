#include "stack.h"

void stack_init(stack_t* s)
{
	s->content = 0;
	s->size = 0;
	s->top = -1;
}

void stack_push(stack_t* s, void* val)
{
	if(s->top + 1 >= s->size)
	{
		(s->size)++;
		if(s->size == 1)
		{
			s->content = (void**)malloc(s->size * sizeof(void*));
		}
		else
		{
			s->content = (void**)realloc(s->content, s->size * sizeof(void*));
		}
	}

	s->top++;
	s->content[s->top] = val;
}

void* stack_top(stack_t* s)
{
	void *ret = 0;
	if(s->top >= 0 && s->content != 0)
	{
		ret = s->content[s->top];
	}
	return ret;
}

void* stack_pop(stack_t* s)
{
	void *ret = 0;
	if(s->top >= 0 && s->content != 0)
	{
		ret = s->content[s->top--];
	}
	return ret;
}

int stack_size(stack_t* s)
{
	return s->top+1;
}

int stack_empty(stack_t* s)
{
	return s->top < 0;
}

void stack_free(stack_t* s)
{
	free(s->content);
	s->content = 0;
	s->size = 0;
	s->top = -1;
}
