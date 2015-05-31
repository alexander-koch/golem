#include "bytecode.h"

// Helper functions

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

void emit_load(list_t* buffer, const char* obj)
{
	value_t* val = value_new_string(obj);
	insert_v1(buffer, OP_LOAD, val);
}

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

void emit_store(list_t* buffer, bool mutable)
{
	if(mutable)
	{
		insert(buffer, OP_MUT_STORE);
	}
	else
	{
		insert(buffer, OP_STORE);
	}
}

void emit_op(list_t* buffer, opcode_t op)
{
	switch(op)
	{
		case OP_ADD:
		{
			insert(buffer, OP_ADD);
			break;
		}
		case OP_SUB:
		{
			insert(buffer, OP_SUB);
			break;
		}
		case OP_MUL:
		{
			insert(buffer, OP_MUL);
			break;
		}
		case OP_DIV:
		{
			insert(buffer, OP_DIV);
			break;
		}
		default: break;
	}
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
		default: break;
	}

	emit_op(buffer, op);
}

void emit_call(list_t* buffer)
{
	insert(buffer, OP_CALL);
}
