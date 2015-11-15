/**
 *  serializer.h
 *  @author Alexander Koch
 *  @desc Converts a list of bytecodes into a *.gvm-file and vice versa.
 */

#ifndef serializer_h
#define serializer_h

#include <stdbool.h>
#include <stdio.h>
#include <adt/vector.h>
#include <vm/bytecode.h>

#define TAG_NUM 1
#define TAG_BOOL 2
#define TAG_STR 3

// File format description
//
// ---- (header)
// uint32_t [magic    ] <-- 0xACCE55 (Access)
// uint32_t [num_codes] <-- number of instructions
// ---- (instruction)
// uint8_t  [opcode   ] <-- operation
// uint8_t  [args     ] <-- arguments
// value*   [values   ] <-- argument values
// ---- (value)
// uint8_t  [tag      ] <-- type tag
// void*    [data     ] <-- direct read as val_t, for objects special condition
// ----
//
// data gets replaced by uint32_t len and char* str, if the type tag is a string
//
// EBNF (sort-of):
// file = header, {instruction}
// instruction = opcode, args, {value}

bool serialize(const char* filename, vector_t* buffer);
bool deserialize(const char* filename, vector_t** out);

#endif
