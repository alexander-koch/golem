#ifndef bytecode_h
#define bytecode_h

#include <core/mem.h>
#include <core/value.h>
#include <lexis/lexer.h>
#include <adt/list.h>

typedef enum
{
	OP_PUSH_INT,
	OP_PUSH_FLOAT,
	OP_PUSH_STRING,
	OP_PUSH_NULL,
	OP_GET_FIELD,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_INVOKE,
	OP_STORE_FIELD
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
void emit_null(list_t* buffer);
void emit_op(list_t* buffer, opcode_t op);
void emit_tok2op(list_t* buffer, token_type_t tok);
void emit_invoke(list_t* buffer, size_t args);
void emit_store_field(list_t* buffer, char* name, bool mutate);
void emit_get_field(list_t* buffer, char* key);

#endif
