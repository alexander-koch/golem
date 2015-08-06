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
	OP_PUSH,
	OP_POP,

	// Store
	OP_STORE,
	OP_LOAD,
	OP_GSTORE,
	OP_GLOAD,

	// Arithmetic
	// Integer
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
	OP_BITNOT,
	OP_IMINUS,

	// Float
	OP_FADD,
	OP_FSUB,
	OP_FMUL,
	OP_FDIV,
	OP_FMINUS,

	// Boolean
	OP_NOT,

	// String
	OP_CONCAT,

	// Special
	OP_SYSCALL,
	OP_INVOKE,
	OP_RET,
	OP_JMP,
	OP_JMPF,
	OP_JMPT,
	OP_ARR,

	// Compare
	OP_BEQ,
	OP_IEQ,
	OP_FEQ,
	OP_STREQ,
	OP_BNE,
	OP_INE,
	OP_FNE,
	OP_STRNE,
	OP_LT,
	OP_GT,
	OP_LE,
	OP_GE,
	OP_BAND,
	OP_BOR,

	// Subscript
	OP_GETSUB,
	OP_SETSUB
} opcode_t;

typedef struct
{
	opcode_t op;
	value_t* v1;
	value_t* v2;
} instruction_t;

const char* op2str(opcode_t code);

void emit_bool(list_t* buffer, bool b);
void emit_int(list_t* buffer, I64 v);
void emit_float(list_t* buffer, F64 f);
void emit_string(list_t* buffer, char* str);
void emit_pop(list_t* buffer);
void emit_op(list_t* buffer, opcode_t op);
bool emit_tok2op(list_t* buffer, token_type_t tok, datatype_t type);
void emit_syscall(list_t* buffer, char* name, size_t args);
void emit_invoke(list_t* buffer, size_t address, size_t args);
void emit_return(list_t* buffer);
void emit_store(list_t* buffer, int address, bool global);
void emit_load(list_t* buffer, int address, bool global);

value_t* emit_jmp(list_t* buffer, int address);
value_t* emit_jmpf(list_t* buffer, int address);
value_t* emit_jmpt(list_t* buffer, int address);


#endif
