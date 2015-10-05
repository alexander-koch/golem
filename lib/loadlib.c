#include "loadlib.h"

#if defined(_WIN32)
#include <windows.h>

void* dl_load(char* path)
{
	return LoadLibraryExA(path, 0, 0);
}

void* dl_func(void* lib, char* name)
{
	return GetProcAddress((HMODULE)lib, name);
}

void dl_unload(void* lib)
{
	FreeLibrary((HMODULE)lib);
}

#else

#include <dlfcn.h>

void* dl_load(char* path)
{
	return dlopen(path, RTLD_NOW | RTLD_LOCAL);
}

void* dl_func(void* lib, char* name)
{
	return dlsym(lib, name);
}

void dl_unload(void* lib)
{
	dlclose(lib);
}

#endif
