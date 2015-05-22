#include "list.h"

void list_init(list_t* list)
{
	list->size = 0;
	list->head = 0;
	list->tail = 0;
}

void list_push(list_t* list, void* val)
{
	list_node_t* node = malloc(sizeof(list_node_t));
	node->next = 0;
	node->val = val;

	if(list->head == 0)
	{
		list->head = node;
	}
	else
	{
		list->tail->next = node;
	}
	list->tail = node;
	list->size++;
}

void* list_pop(list_t* list)
{
	assert(list->head != 0);
	list_node_t* rem = list->head;
	void* val = rem->val;

	list->head = list->head->next;
	free(rem);
	list->size--;
	return val;
}

void* list_get(list_t* list, size_t index)
{
	list_node_t* node = list->head;
	int i;
	for(i = 1; i <= index; i++)
	{
		assert(node);
		node = node->next;
	}

	return node->val;
}

size_t list_size(list_t* list)
{
	return list->size;
}

void list_free(list_t* list)
{
	list_node_t* curr;
	while(list->head != 0)
	{
		curr = list->head;
		list->head = list->head->next;
		free(curr);
		curr = 0;
	}
	list->head = 0;
}

list_iterator_t* list_iterator_create(list_t* list)
{
	list_iterator_t* iter = malloc(sizeof(*iter));
	if(!iter) return iter;
	iter->ptr = list->head;
	iter->list = list;
	return iter;
}

void* list_iterator_next(list_iterator_t* iter)
{
	list_node_t* ptr = iter->ptr;
	iter->ptr = ptr->next;
	return ptr->val;
}

int list_iterator_end(list_iterator_t* iter)
{
	return !iter->ptr;
}

void list_iterator_free(list_iterator_t* iter)
{
	free(iter);
}
