#ifndef hashmap_h
#define hashmap_h

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <core/mem.h>
#include <core/api.h>

#define HMAP_MISSING -3
#define HMAP_FULL -2
#define HMAP_MEM -1
#define HMAP_OK 0

typedef struct bucket_t
{
	char* key;
	bool use;
	void* data;
} bucket_t;

typedef struct hashmap_t
{
	size_t table_size;
	size_t size;
	bucket_t* data;
} hashmap_t;

hashmap_t* hashmap_new();
int hashmap_set(hashmap_t* hashmap, char* key, void* value);
int hashmap_get(hashmap_t* hashmap, char* key, void** value);
size_t hashmap_length(hashmap_t* hashmap);
void hashmap_free(hashmap_t* hashmap);

#endif