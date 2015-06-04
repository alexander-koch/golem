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
	OP_STORE,
	OP_MUT_STORE,
	OP_ASSIGN,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_LOAD,
	OP_CALL,
	OP_EQUAL,
	OP_NEQUAL,
	OP_LESS,
	OP_GREATER,
	OP_LEQUAL,
	OP_GEQUAL,
	OP_ARRAY
} opcode_t;

typedef struct
{
	opcode_t op;
	value_t* v1;
	value_t* v2;
} instruction_t;

const char* op2str(opcode_t code);

void emit_load(list_t* buffer, const char* obj);
void emit_i64(list_t* buffer, I64 v);
void emit_f64(list_t* buffer, F64 f);
void emit_string(list_t* buffer, char* str);
void emit_store(list_t* buffer, bool mutable, char* key);
void emit_op(list_t* buffer, opcode_t op);
void emit_tok2op(list_t* buffer, token_type_t tok);
void emit_call(list_t* buffer, char* key, I64 args);
void emit_array(list_t* buffer, I64 size);

#endif
