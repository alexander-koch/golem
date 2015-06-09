#include "bytecode.h"

// Helper functions

const char* op2str(opcode_t code)
{
	switch(code)
	{
		case OP_PUSH_INT: return "push_int";
		case OP_PUSH_FLOAT: return "push_float";
		case OP_PUSH_STRING: return "push_string";
		case OP_PUSH_NULL: return "push_null";
		case OP_GET_FIELD: return "get_field";
		case OP_ADD: return "add";
		case OP_SUB: return "sub";
		case OP_MUL: return "mul";
		case OP_DIV: return "div";
		case OP_MOD: return "mod";
		case OP_INVOKE: return "invoke";
		case OP_STORE_FIELD: return "store_field";
		default: break;
	}
	return "unknown";
}

instruction_t* instruction_new(opcode_t op)
{
	instruction_t* ins = malloc(sizeof(*ins));
	ins->op = op;
	ins->v1 = 0;
	ins->v2 = 0;
	return ins;
}

void push(list_t* buffer, instruction_t* ins)
{
	list_push(buffer, ins);
}

void insert(list_t* buffer, opcode_t op)
{
	push(buffer, instruction_new(op));
}

void insert_v1(list_t* buffer, opcode_t op, value_t* v1)
{
	instruction_t* ins = instruction_new(op);
	ins->v1 = v1;
	push(buffer, ins);
}

void insert_v2(list_t* buffer, opcode_t op, value_t* v1, value_t* v2)
{
	instruction_t* ins = instruction_new(op);
	ins->v1 = v1;
	ins->v2 = v2;
	push(buffer, ins);
}

// Main functions

void emit_i64(list_t* buffer, I64 v)
{
	value_t* val = value_new_int(v);
	insert_v1(buffer, OP_PUSH_INT, val);
}

void emit_f64(list_t* buffer, F64 f)
{
	value_t* val = value_new_float(f);
	insert_v1(buffer, OP_PUSH_FLOAT, val);
}

void emit_string(list_t* buffer, char* str)
{
	value_t* val = value_new_string(str);
	insert_v1(buffer, OP_PUSH_STRING, val);
}

void emit_null(list_t* buffer)
{
	value_t* val = value_new_null();
	insert_v1(buffer, OP_PUSH_NULL, val);
}

void emit_op(list_t* buffer, opcode_t op)
{
	insert(buffer, op);
}

void emit_tok2op(list_t* buffer, token_type_t tok)
{
	opcode_t op;
	switch(tok)
	{
		case TOKEN_ADD:
		{
			op = OP_ADD;
			break;
		}
		case TOKEN_SUB:
		{
			op = OP_SUB;
			break;
		}
		case TOKEN_MUL:
		{
			op = OP_MUL;
			break;
		}
		case TOKEN_DIV:
		{
			op = OP_DIV;
			break;
		}
		case TOKEN_MOD:
		{
			op = OP_MOD;
			break;
		}
		default: break;
	}

	emit_op(buffer, op);
}

void emit_invoke(list_t* buffer, size_t args)
{
	value_t* v1 = value_new_int(args);
	insert_v1(buffer, OP_INVOKE, v1);
}

void emit_store_field(list_t* buffer, char* name, bool mutate)
{
	value_t* val = value_new_string(name);
	value_t* m = value_new_bool(mutate);
	insert_v2(buffer, OP_STORE_FIELD, val, m);
}

void emit_get_field(list_t* buffer, char* key)
{
	value_t* val = value_new_string(key);
	insert_v1(buffer, OP_GET_FIELD, val);
}
