#ifndef bytecode_h
#define bytecode_h

#include <core/mem.h>
#include <lexis/lexer.h>
#include <adt/list.h>
#include <vm/value.h>

typedef enum
{
	// Basic stack
	OP_ICONST,
	OP_FCONST,
	OP_POP,

	// Store
	OP_STORE,
	OP_LOAD,

	// Arithmetic
	OP_IADD,
	OP_ISUB,
	OP_IMUL,
	OP_IDIV,
	OP_MOD,
	OP_BITL,
	OP_BITR,
	OP_BITAND,
	OP_BITOR,
	OP_BITXOR,

	// Special
	OP_INVOKE,
	OP_JMP,
	OP_JMPF,
	OP_JMPT,

	// Compare
	OP_IEQ,
	OP_INE,
	OP_ILT
} opcode_t;

typedef struct
{
	opcode_t op;
	value_t* v1;
	value_t* v2;
} instruction_t;

const char* op2str(opcode_t code);

void emit_int(list_t* buffer, I64 v);
void emit_float(list_t* buffer, F64 f);
void emit_pop(list_t* buffer);
void emit_op(list_t* buffer, opcode_t op);
void emit_tok2op(list_t* buffer, token_type_t tok);
void emit_invoke(list_t* buffer, char* str, size_t args);
void emit_store(list_t* buffer, char* name);
void emit_load(list_t* buffer, char* key);

value_t* emit_jmp(list_t* buffer, int address);
value_t* emit_jmpf(list_t* buffer, int address);
value_t* emit_jmpt(list_t* buffer, int address);


#endif
