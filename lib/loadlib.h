#ifndef loadlib_h
#define loadlib_h

// External method definition
typedef int (*InitFunc)(void*);
typedef void* (*SymbolFunc)();

void* dl_load(char* path);
void* dl_func(void* lib, char* name);
void dl_unload(void* lib);

#endif
