#ifndef bytecode_h
#define bytecode_h

#include <core/mem.h>
#include <lexis/lexer.h>
#include <adt/list.h>
#include <vm/value.h>

typedef enum
{
	OP_PUSH_INT,
	OP_PUSH_FLOAT,
	OP_PUSH_STRING,
	OP_GET_FIELD,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_INVOKE,
	OP_STORE_FIELD,
	OP_BEGIN_FUNC,
	OP_SCOPE_END,
	OP_JMP,
	OP_JMPF,
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
void emit_jmp(list_t* buffer, int address);
value_t* emit_jmpf(list_t* buffer, int address);

void emit_begin_func(list_t* buffer, char* name, size_t params);
void emit_scope_end(list_t* buffer);

#endif
