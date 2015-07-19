#include "bytecode.h"

// Helper functions

const char* op2str(opcode_t code)
{
	switch(code)
	{
		case OP_PUSH_INT: return "push_int";
		case OP_PUSH_FLOAT: return "push_float";
		case OP_PUSH_STRING: return "push_string";
		case OP_STORE_FIELD: return "store_field";
		case OP_GET_FIELD: return "get_field";
		case OP_ADD: return "add";
		case OP_SUB: return "sub";
		case OP_MUL: return "mul";
		case OP_DIV: return "div";
		case OP_MOD: return "mod";
		case OP_BITL: return "bit_l";
		case OP_BITR: return "bit_r";
		case OP_BITAND: return "bit_and";
		case OP_BITOR: return "bit_or";
		case OP_BITXOR: return "bit_xor";
		case OP_INVOKE: return "invoke";
		case OP_PUSH_SCOPE: return "push_scope";
		case OP_POP_SCOPE: return "pop_scope";
		case OP_JMP: return "jump";
		case OP_JMPF: return "jump_false";
		case OP_EQUAL: return "equal";
		case OP_LESS: return "less";
		default: return "unknown";
	}
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
		case TOKEN_BITLSHIFT:
		{
			op = OP_BITL;
			break;
		}
		case TOKEN_BITRSHIFT:
		{
			op = OP_BITR;
			break;
		}
		case TOKEN_BITAND:
		{
			op = OP_BITAND;
			break;
		}
		case TOKEN_BITOR:
		{
			op = OP_BITOR;
			break;
		}
		case TOKEN_BITXOR:
		{
			op = OP_BITXOR;
			break;
		}
		case TOKEN_EQUAL:
		{
			op = OP_EQUAL;
			break;
		}
		case TOKEN_LESS:
		{
			op = OP_LESS;
			break;
		}
		default:
		{
			op = -1;
			break;
		}
	}

	emit_op(buffer, op);
}

void emit_invoke(list_t* buffer, char* str, size_t args)
{
	value_t* v1 = value_new_string(str);
	value_t* v2 = value_new_int(args);
	insert_v2(buffer, OP_INVOKE, v1, v2);
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

void emit_push_scope(list_t* buffer, char* name, size_t params)
{
	value_t* key = value_new_string(name);
	value_t* args = value_new_int(params);
	insert_v2(buffer, OP_PUSH_SCOPE, key, args);
}

void emit_pop_scope(list_t* buffer)
{
	insert(buffer, OP_POP_SCOPE);
}

value_t* emit_jmp(list_t* buffer, int address)
{
	value_t* val = value_new_int(address);
	insert_v1(buffer, OP_JMP, val);
	return val;
}

value_t* emit_jmpf(list_t* buffer, int address)
{
	value_t* val = value_new_int(address);
	insert_v1(buffer, OP_JMPF, val);
	return val;
}
