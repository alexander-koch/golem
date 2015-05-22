#ifndef queue_h
#define queue_h

#include <stddef.h>
#include <assert.h>
#include <core/mem.h>

typedef struct queue_node_t
{
	struct queue_node_t* next;
	void* val;
} queue_node_t;

typedef struct
{
	size_t size;
	queue_node_t* head;
	queue_node_t* tail;
} queue_t;

void queue_init(queue_t* qu);
void queue_push(queue_t* qu, void* val);
void* queue_head(queue_t* qu);
void* queue_pop(queue_t* qu);
int queue_empty(queue_t* qu);
size_t queue_size(queue_t* qu);
void queue_free(queue_t* qu);

#endif
