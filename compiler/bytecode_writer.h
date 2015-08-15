#ifndef bytecode_writer_h
#define bytecode_writer_h

#include <stdio.h>
#include <adt/list.h>
#include <vm/bytecode.h>

bool write_bytecode(const char* filename, list_t* instructions);
bool read_bytecode(const char* filename, list_t** out);

#endif
