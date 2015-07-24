#include "bytecode.h"

// Helper functions

const char* op2str(opcode_t code)
{
	switch(code)
	{
		case OP_ICONST: return "iconst";
		case OP_FCONST: return "fconst";
		case OP_SCONST: return "sconst";
		case OP_POP: return "pop";
		case OP_STORE: return "store";
		case OP_LOAD: return "load";
		case OP_IADD: return "iadd";
		case OP_ISUB: return "isub";
		case OP_IMUL: return "imul";
		case OP_IDIV: return "idiv";
		case OP_MOD: return "mod";
		case OP_BITL: return "bit_l";
		case OP_BITR: return "bit_r";
		case OP_BITAND: return "bit_and";
		case OP_BITOR: return "bit_or";
		case OP_BITXOR: return "bit_xor";
		case OP_INVOKE: return "invoke";
		case OP_JMP: return "jmp";
		case OP_JMPF: return "jmpf";
		case OP_JMPT: return "jmpt";
		case OP_IEQ: return "ieq";
		case OP_INE: return "ine";
		case OP_ILT: return "ilt";
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

void emit_int(list_t* buffer, I64 v)
{
	value_t* val = value_new_int(v);
	insert_v1(buffer, OP_ICONST, val);
}

void emit_float(list_t* buffer, F64 f)
{
	value_t* val = value_new_float(f);
	insert_v1(buffer, OP_FCONST, val);
}

void emit_string(list_t* buffer, char* str)
{
	value_t* val = value_new_string(str);
	insert_v1(buffer, OP_SCONST, val);
}

void emit_pop(list_t* buffer)
{
	insert(buffer, OP_POP);
}

void emit_op(list_t* buffer, opcode_t op)
{
	insert(buffer, op);
}

void emit_tok2op(list_t* buffer, token_type_t tok)
{
	// Test type first!

	opcode_t op;
	switch(tok)
	{
		case TOKEN_ADD:
		{
			op = OP_IADD;
			break;
		}
		case TOKEN_SUB:
		{
			op = OP_ISUB;
			break;
		}
		case TOKEN_MUL:
		{
			op = OP_IMUL;
			break;
		}
		case TOKEN_DIV:
		{
			op = OP_IDIV;
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
			op = OP_IEQ;
			break;
		}
		case TOKEN_NEQUAL:
		{
			op = OP_INE;
			break;
		}
		case TOKEN_LESS:
		{
			op = OP_ILT;
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

void emit_store(list_t* buffer, int address)
{
	value_t* val = value_new_int(address);
	insert_v1(buffer, OP_STORE, val);
}

void emit_load(list_t* buffer, int address)
{
	value_t* val = value_new_int(address);
	insert_v1(buffer, OP_LOAD, val);
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

value_t* emit_jmpt(list_t* buffer, int address)
{
	value_t* val = value_new_int(address);
	insert_v1(buffer, OP_JMPT, val);
	return val;
}
