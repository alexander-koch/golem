#ifndef native_h
#define native_h

#if defined(_WIN32)
#define OS_SHARED_LIB_FORMAT_STR "lib%s.dll"
#else
#define OS_SHARED_LIB_FORMAT_STR "lib%s.so"
#endif

typedef struct {
	char* name;
	void* handle;
} shared_lib;

void* dl_load(char* path);
void* dl_sym(void* lib, char* name);
void dl_unload(void* lib);

char* create_system_library_name(char* lib);

#endif
