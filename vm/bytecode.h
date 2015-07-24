#ifndef bytecode_h
#define bytecode_h

#include <core/mem.h>
#include <lexis/lexer.h>
#include <adt/list.h>
#include <vm/value.h>

typedef enum
{
	// Basic stack
	OP_ICONST = 1,
	OP_FCONST = 2,
	OP_SCONST = 3,
	OP_POP = 4,

	// Store
	OP_STORE = 5,
	OP_LOAD = 6,

	// Arithmetic
	OP_IADD = 7,
	OP_ISUB = 8,
	OP_IMUL = 9,
	OP_IDIV = 10,
	OP_MOD = 11,
	OP_BITL = 12,
	OP_BITR = 13,
	OP_BITAND = 14,
	OP_BITOR = 15,
	OP_BITXOR = 16,

	// Special
	OP_INVOKE = 17,
	OP_JMP = 18,
	OP_JMPF = 19,
	OP_JMPT = 20,

	// Compare
	OP_IEQ = 21,
	OP_INE = 22,
	OP_ILT = 23
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
void emit_string(list_t* buffer, char* str);
void emit_pop(list_t* buffer);
void emit_op(list_t* buffer, opcode_t op);
void emit_tok2op(list_t* buffer, token_type_t tok);
void emit_invoke(list_t* buffer, char* str, size_t args);
void emit_store(list_t* buffer, int address);
void emit_load(list_t* buffer, int address);

value_t* emit_jmp(list_t* buffer, int address);
value_t* emit_jmpf(list_t* buffer, int address);
value_t* emit_jmpt(list_t* buffer, int address);


#endif
