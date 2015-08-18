#ifndef bytecode_writer_h
#define bytecode_writer_h

#include <stdio.h>
#include <adt/vector.h>
#include <vm/bytecode.h>

bool write_bytecode(const char* filename, vector_t* instructions);
bool read_bytecode(const char* filename, vector_t** out);

#endif
