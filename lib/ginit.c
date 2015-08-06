
#define ginit_c

#include <stdio.h>
#include <vm/vm.h>
#include <adt/hashmap.h>

typedef struct LibraryDef
{
	const char* name;
	int (*func)(hashmap_t*);
} LibraryDef;

extern int open_mathlib(hashmap_t*);
extern int open_iolib(hashmap_t*);

const LibraryDef loadedlibs[] = {
	{"math", open_mathlib},
	{"io", open_iolib},
	{0, 0}
};

void open_libs(hashmap_t* symbols)
{
	const LibraryDef* lib;
	for(lib = loadedlibs; lib->func; lib++)  {
		hashmap_t* sublib = hashmap_new();
		lib->func(sublib);
		hashmap_set(symbols, (char*)lib->name, sublib);
	}
}
