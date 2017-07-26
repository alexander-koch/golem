/**
 * bytecode.h
 * Copyright (C) 2017 Alexander Koch
 * Bytecode definition for the VM
 *
 * An instruction is defined as a c-struct
 * with one opcode and two optional val_t generic values.
 * (See val.h / val.c for more info.)
 *
 * instruction
 * |- op
 * |- v0
 * |- v1
 *
 * Also the compiler-specific helper functions are defined below.
 * For instruction lists a vector_t should be used (see adt/vector.h).
 */

#ifndef bytecode_h
#define bytecode_h

#include <core/mem.h>
#include <lexis/lexer.h>
#include <adt/vector.h>
#include <parser/ast.h>
#include <vm/val.h>

typedef enum {
    // Basic stack
    OP_HLT                     = 0x00,
    OP_PUSH                    = 0x01,
    OP_POP                     = 0x02,

    // Store
    OP_STORE                   = 0x03,
    OP_LOAD                    = 0x04,
    OP_GSTORE                  = 0x05,
    OP_GLOAD                   = 0x06,
    OP_LDARG0                  = 0x07,
    OP_SETARG0                 = 0x08,

    // Arithmetic
    // Integer
    OP_IADD                    = 0x09,
    OP_ISUB                    = 0x0a,
    OP_IMUL                    = 0x0b,
    OP_IDIV                    = 0x0c,
    OP_MOD                     = 0x0d,
    OP_BITL                    = 0x0e,
    OP_BITR                    = 0x0f,
    OP_BITAND                  = 0x10,
    OP_BITOR                   = 0x11,
    OP_BITXOR                  = 0x12,
    OP_BITNOT                  = 0x13,
    OP_IMINUS                  = 0x14,
    OP_I2F                     = 0x15,

    // Float
    OP_FADD                    = 0x16,
    OP_FSUB                    = 0x17,
    OP_FMUL                    = 0x18,
    OP_FDIV                    = 0x19,
    OP_FMINUS                  = 0x1a,
    OP_F2I                     = 0x1b,

    // Boolean
    OP_NOT                     = 0x1c,
    OP_B2I                     = 0x1d,

    // Special
    OP_SYSCALL                 = 0x1e,
    OP_INVOKE                  = 0x1f,
    OP_RESERVE                 = 0x20,
    OP_RET                     = 0x21,
    OP_RETVIRTUAL              = 0x22,
    OP_JMP                     = 0x23,
    OP_JMPF                    = 0x24,
    OP_ARR                     = 0x25,
    OP_STR                     = 0x26,
    OP_LDLIB                   = 0x27,
    OP_TOSTR                   = 0x28,

    // Compare
    OP_BEQ,
    OP_IEQ,
    OP_FEQ,
    OP_BNE,
    OP_INE,
    OP_FNE,

    // Integer
    OP_ILT,
    OP_IGT,
    OP_ILE,
    OP_IGE,

    // Float
    OP_FLT,
    OP_FGT,
    OP_FLE,
    OP_FGE,

    OP_BAND,
    OP_BOR,

    // Subscript
    OP_GETSUB,
    OP_SETSUB,
    OP_LEN,
    OP_APPEND,
    OP_CONS,

    // Upval
    OP_UPVAL,
    OP_UPSTORE,

    // Class
    OP_CLASS,
    OP_SETFIELD,
    OP_GETFIELD
} opcode_t;

// Instruction definition
typedef struct {
    opcode_t op;
    val_t v1;
    val_t v2;
} instruction_t;

// Helper functions
const char* op2str(opcode_t code);

/**
 * Insert one opcode and one or two values.
 */
void insert(vector_t* buffer, opcode_t op);
void insert_v1(vector_t* buffer, opcode_t op, val_t v1);
void insert_v2(vector_t* buffer, opcode_t op, val_t v1, val_t v2);

/**
 * Helper functions for creating bytecode
 */
void emit_bool(vector_t* buffer, bool b);
void emit_int(vector_t* buffer, int v);
void emit_float(vector_t* buffer, double f);
void emit_string(vector_t* buffer, char* str);
void emit_char(vector_t* buffer, char c);
void emit_pop(vector_t* buffer);
void emit_op(vector_t* buffer, opcode_t op);
bool emit_tok2op(vector_t* buffer, token_type_t tok, datatype_t* type);
void emit_syscall(vector_t* buffer, size_t index);
void emit_invoke(vector_t* buffer, size_t address, size_t args);
void emit_return(vector_t* buffer);
void emit_store(vector_t* buffer, int address, bool global);
void emit_load(vector_t* buffer, int address, bool global);
void emit_load_upval(vector_t* buffer, int depth, int address);
void emit_store_upval(vector_t* buffer, int depth, int address);
void emit_class_setfield(vector_t* buffer, int address);
void emit_class_getfield(vector_t* buffer, int address);
void emit_reserve(vector_t* buffer, size_t sz);
void emit_string_merge(vector_t* buffer, size_t sz);
void emit_array_merge(vector_t* buffer, size_t sz);
void emit_dynlib(vector_t* buffer, char* name);

/**
 * Functions that return a pointer.
 * Jumps return a value pointer.
 * Modify it, if you want to change the address afterwards.
 * Same applies to the class field count for emit_class.
 */
val_t* emit_class(vector_t* buffer, int fields);
val_t* emit_jmp(vector_t* buffer, int address);
val_t* emit_jmpf(vector_t* buffer, int address);

/**
 * Free a list/vector of instructions.
 * Always use this.
 */
void bytecode_buffer_free(vector_t* buffer);

#endif
