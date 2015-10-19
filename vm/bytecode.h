#ifndef bytecode_h
#define bytecode_h

#include <core/mem.h>
#include <lexis/lexer.h>
#include <adt/vector.h>
#include <parser/ast.h>
#include <vm/val.h>

typedef enum
{
	// Basic stack
	OP_HLT 					= 0x00,
	OP_PUSH					= 0x01,
	OP_POP					= 0x02,

	// Store
	OP_STORE				= 0x03,
	OP_LOAD					= 0x04,
	OP_GSTORE				= 0x05,
	OP_GLOAD				= 0x06,
	OP_LDARG0				= 0x07,
	OP_SETARG0				= 0x08,

	// Arithmetic
	// Integer
	OP_IADD					= 0x09,
	OP_ISUB					= 0x0a,
	OP_IMUL					= 0x0b,
	OP_IDIV					= 0x0c,
	OP_MOD					= 0x0d,
	OP_BITL					= 0x0e,
	OP_BITR					= 0x0f,
	OP_BITAND				= 0x10,
	OP_BITOR				= 0x11,
	OP_BITXOR 				= 0x12,
	OP_BITNOT				= 0x13,
	OP_IMINUS				= 0x14,
	OP_I2F					= 0x15,

	// Float
	OP_FADD					= 0x16,
	OP_FSUB					= 0x17,
	OP_FMUL,
	OP_FDIV,
	OP_FMINUS,

	// Boolean
	OP_NOT,

	// Special
	OP_SYSCALL,
	OP_INVOKE,
	OP_INVOKEVIRTUAL,
	OP_RESERVE,
	OP_RET,
	OP_RETVIRTUAL,
	OP_JMP,
	OP_JMPF,
	OP_ARR,
	OP_STR,
	OP_LDLIB,

	// Compare
	OP_BEQ,
	OP_IEQ,
	OP_FEQ,
	OP_CEQ,
	OP_BNE,
	OP_INE,
	OP_FNE,
	OP_CNE,

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

typedef struct
{
	opcode_t op;
	val_t v1;
	val_t v2;
} instruction_t;

const char* op2str(opcode_t code);

void insert(vector_t* buffer, opcode_t op);
void insert_v1(vector_t* buffer, opcode_t op, val_t v1);
void insert_v2(vector_t* buffer, opcode_t op, val_t v1, val_t v2);

void emit_bool(vector_t* buffer, bool b);
void emit_int(vector_t* buffer, I64 v);
void emit_float(vector_t* buffer, F64 f);
void emit_string(vector_t* buffer, char* str);
void emit_char(vector_t* buffer, char c);
void emit_pop(vector_t* buffer);
void emit_op(vector_t* buffer, opcode_t op);
bool emit_tok2op(vector_t* buffer, token_type_t tok, datatype_t type);
void emit_syscall(vector_t* buffer, size_t index, size_t args);
void emit_invoke(vector_t* buffer, size_t address, size_t args);
void emit_return(vector_t* buffer);
void emit_store(vector_t* buffer, int address, bool global);
void emit_load(vector_t* buffer, int address, bool global);
void emit_load_upval(vector_t* buffer, int depth, int address);
void emit_store_upval(vector_t* buffer, int depth, int address);
void emit_class_setfield(vector_t* buffer, int address);
void emit_class_getfield(vector_t* buffer, int address);
void emit_lib_load(vector_t* buffer, char* name);
void emit_reserve(vector_t* buffer, size_t sz);
void emit_string_merge(vector_t* buffer, size_t sz);
void emit_array_merge(vector_t* buffer, size_t sz);

val_t* emit_jmp(vector_t* buffer, int address);
val_t* emit_jmpf(vector_t* buffer, int address);

#endif
