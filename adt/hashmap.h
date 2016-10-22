/**
 * hashmap.h
 * @author Alexander Koch
 * @desc Hashmap implementation
 */

#ifndef hashmap_h
#define hashmap_h

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <core/mem.h>

// Tags for return codes
#define HMAP_MISSING -3
#define HMAP_FULL -2
#define HMAP_MEM -1
#define HMAP_OK 0

typedef int (*HashForeachFunc)(void* data, void* arg);

typedef struct bucket_t {
    const char* key;
    bool use;
    void* data;
} bucket_t;

typedef struct hashmap_t {
    size_t table_size;
    size_t size;
    bucket_t* data;
} hashmap_t;

hashmap_t* hashmap_new(void);
int hashmap_set(hashmap_t* hashmap, const char* key, void* value);
int hashmap_get(hashmap_t* hashmap, const char* key, void** value);
void* hashmap_find(hashmap_t* hashmap, const char* key);
int hashmap_foreach(hashmap_t* hashmap, HashForeachFunc func, void* arg);
size_t hashmap_length(hashmap_t* hashmap);
void hashmap_free(hashmap_t* hashmap);

// Hashmap iteration
typedef struct hashmap_iterator_t {
    hashmap_t* hmap;
    size_t idx;
} hashmap_iterator_t;

hashmap_iterator_t* hashmap_iterator_create(hashmap_t* hashmap);
void* hashmap_iterator_next(hashmap_iterator_t* iterator);
bool hashmap_iterator_end(hashmap_iterator_t* iterator);
void hashmap_iterator_free(hashmap_iterator_t* iterator);

#endif
