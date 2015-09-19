/** list.h
 *	@author Alexander Koch
 * 	@desc List implementation based on a queue
 */

#ifndef list_h
#define list_h

#include <stddef.h>
#include <assert.h>
#include <core/mem.h>

typedef struct list_node_t
{
	struct list_node_t* next;
	void* val;
} list_node_t;

typedef struct list_t
{
	size_t size;
	list_node_t* head;
	list_node_t* tail;
} list_t;

typedef struct list_iterator_t
{
	list_t* list;
	list_node_t* ptr;
} list_iterator_t;

list_t* list_new();
void* list_top(list_t* list);
void list_push(list_t* list, void* val);
void* list_pop(list_t* list);
void* list_pop_back(list_t* list);
void list_append(list_t* l1, list_t* l2);
void* list_get(list_t* list, size_t index);
size_t list_size(list_t* list);
void list_free(list_t* list);

list_iterator_t* list_iterator_create(list_t* list);
void* list_iterator_next(list_iterator_t* iter);
int list_iterator_end(list_iterator_t* iter);
void list_iterator_free(list_iterator_t* iter);

#endif
