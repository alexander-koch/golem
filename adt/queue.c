#include "queue.h"

void queue_init(queue_t* qu)
{
	qu->size = 0;
	qu->head = 0;
	qu->tail = 0;
}

void queue_push(queue_t* qu, void* val)
{
	queue_node_t* node = malloc(sizeof(queue_node_t));
	node->next = 0;
	node->val = val;

	if(qu->head == 0)
	{
		qu->head = node;
	}
	else
	{
		qu->tail->next = node;
	}
	qu->tail = node;
	qu->size++;
}

void* queue_head(queue_t* qu)
{
	assert(qu->head != 0);
	return qu->head->val;
}

void* queue_pop(queue_t* qu)
{
	assert(qu->head != 0);
	queue_node_t* rem = qu->head;
	void* val = rem->val;

	qu->head = qu->head->next;
	free(rem);
	qu->size--;
	return val;
}

int queue_empty(queue_t* qu)
{
	return qu->size == 0;
}

size_t queue_size(queue_t* qu)
{
	return qu->size;
}

void queue_free(queue_t* qu)
{
	queue_node_t* curr;
	while(qu->head != 0)
	{
		curr = qu->head;
		qu->head = qu->head->next;
		free(curr);
		curr = 0;
	}
	qu->head = 0;
}
