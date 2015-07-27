#include "bytecode.h"

// Helper functions

const char* op2str(opcode_t code)
{
	switch(code)
	{
		case OP_PUSH: return "push";
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
		case OP_CONCAT: return "concat";
		case OP_SYSCALL: return "syscall";
		case OP_INVOKE: return "invoke";
		case OP_RET: return "ret";
		case OP_JMP: return "jmp";
		case OP_JMPF: return "jmpf";
		case OP_JMPT: return "jmpt";
		case OP_IEQ: return "ieq";
		case OP_STREQ: return "streq";
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

void push_ins(list_t* buffer, instruction_t* ins)
{
	list_push(buffer, ins);
}

void insert(list_t* buffer, opcode_t op)
{
	push_ins(buffer, instruction_new(op));
}

void insert_v1(list_t* buffer, opcode_t op, value_t* v1)
{
	instruction_t* ins = instruction_new(op);
	ins->v1 = v1;
	push_ins(buffer, ins);
}

void insert_v2(list_t* buffer, opcode_t op, value_t* v1, value_t* v2)
{
	instruction_t* ins = instruction_new(op);
	ins->v1 = v1;
	ins->v2 = v2;
	push_ins(buffer, ins);
}

// Main functions

void emit_int(list_t* buffer, I64 v)
{
	value_t* val = value_new_int(v);
	insert_v1(buffer, OP_PUSH, val);
}

void emit_float(list_t* buffer, F64 f)
{
	value_t* val = value_new_float(f);
	insert_v1(buffer, OP_PUSH, val);
}

void emit_string(list_t* buffer, char* str)
{
	value_t* val = value_new_string(str);
	insert_v1(buffer, OP_PUSH, val);
}

void emit_pop(list_t* buffer)
{
	insert(buffer, OP_POP);
}

void emit_op(list_t* buffer, opcode_t op)
{
	insert(buffer, op);
}

opcode_t getOp(token_type_t tok, datatype_t type)
{
	switch(tok)
	{
		case TOKEN_ADD:
		{
			if(type == DATA_INT) return OP_IADD;
			if(type == DATA_STRING) return OP_CONCAT;
			return -1;
		}
		case TOKEN_SUB: return OP_ISUB;
		case TOKEN_MUL: return OP_IMUL;
		case TOKEN_DIV: return OP_IMUL;
		case TOKEN_MOD: return OP_MOD;
		case TOKEN_BITLSHIFT: return OP_BITL;
		case TOKEN_BITRSHIFT: return OP_BITR;
		case TOKEN_BITAND: return OP_BITAND;
		case TOKEN_BITOR: return OP_BITOR;
		case TOKEN_BITXOR: return OP_BITXOR;
		case TOKEN_EQUAL:
		{
			if(type == DATA_INT) return OP_IEQ;
			if(type == DATA_STRING) return OP_STREQ;
			return -1;
		}
		case TOKEN_NEQUAL: return OP_INE;
		case TOKEN_LESS: return OP_ILT;
		default:
		{
			return -1;
		}
	}
}

void emit_tok2op(list_t* buffer, token_type_t tok, datatype_t type)
{
	opcode_t op = getOp(tok, type);
	emit_op(buffer, op);
}

void emit_syscall(list_t* buffer, char* name, size_t args)
{
	value_t* v1 = value_new_string(name);
	value_t* v2 = value_new_int(args);
	insert_v2(buffer, OP_SYSCALL, v1, v2);
}

void emit_invoke(list_t* buffer, size_t address, size_t args)
{
	value_t* v1 = value_new_int(address);
	value_t* v2 = value_new_int(args);
	insert_v2(buffer, OP_INVOKE, v1, v2);
}

void emit_return(list_t* buffer)
{
	insert(buffer, OP_RET);
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
