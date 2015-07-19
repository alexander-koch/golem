#ifndef bytecode_h
#define bytecode_h

#include <core/mem.h>
#include <lexis/lexer.h>
#include <adt/list.h>
#include <vm/value.h>

typedef enum
{
	// Push
	OP_PUSH_INT,
	OP_PUSH_FLOAT,

	// Store
	OP_PUSH_STRING,
	OP_STORE_FIELD,
	OP_GET_FIELD,
	OP_PUSH_SCOPE,
	OP_POP_SCOPE,

	// Arithmetic
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
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

	// Compare
	OP_EQUAL,
	OP_LESS
} opcode_t;

typedef struct
{
	opcode_t op;
	value_t* v1;
	value_t* v2;
} instruction_t;

const char* op2str(opcode_t code);

void emit_i64(list_t* buffer, I64 v);
void emit_f64(list_t* buffer, F64 f);
void emit_string(list_t* buffer, char* str);
void emit_op(list_t* buffer, opcode_t op);
void emit_tok2op(list_t* buffer, token_type_t tok);
void emit_invoke(list_t* buffer, char* str, size_t args);
void emit_store_field(list_t* buffer, char* name, bool mutate);
void emit_get_field(list_t* buffer, char* key);
void emit_push_scope(list_t* buffer, char* name, size_t params);
void emit_pop_scope(list_t* buffer);

value_t* emit_jmp(list_t* buffer, int address);
value_t* emit_jmpf(list_t* buffer, int address);


#endif
