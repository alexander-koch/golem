#ifndef bytecode_h
#define bytecode_h

#include <core/mem.h>
#include <lexis/lexer.h>
#include <adt/list.h>
#include <vm/value.h>
#include <parser/ast.h>

typedef enum
{
	// Basic stack
	OP_PUSH = 1,
	OP_POP = 2,

	// Store
	OP_STORE = 3,
	OP_LOAD = 4,

	// Arithmetic
	OP_IADD = 5,
	OP_ISUB = 6,
	OP_IMUL = 7,
	OP_IDIV = 8,
	OP_MOD = 9,
	OP_BITL = 10,
	OP_BITR = 11,
	OP_BITAND = 12,
	OP_BITOR = 13,
	OP_BITXOR = 14,
	OP_CONCAT = 15,

	// Special
	OP_INVOKE = 16,
	OP_JMP = 17,
	OP_JMPF = 18,
	OP_JMPT = 19,

	// Compare
	OP_IEQ = 20,
	OP_STREQ = 21,
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
void emit_tok2op(list_t* buffer, token_type_t tok, datatype_t type);
void emit_invoke(list_t* buffer, char* str, size_t args);
void emit_store(list_t* buffer, int address);
void emit_load(list_t* buffer, int address);

value_t* emit_jmp(list_t* buffer, int address);
value_t* emit_jmpf(list_t* buffer, int address);
value_t* emit_jmpt(list_t* buffer, int address);


#endif
