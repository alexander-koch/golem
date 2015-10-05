#include "gbytecode.h"

// Helper functions

const char* op2str(opcode_t code)
{
	switch(code)
	{
		case OP_PUSH: return "push";
		case OP_POP: return "pop";
		case OP_HLT: return "hlt";
		case OP_STORE: return "store";
		case OP_LOAD: return "load";
		case OP_GSTORE: return "gstore";
		case OP_GLOAD: return "gload";
		case OP_LDARG0: return "ldarg0";
		case OP_SETARG0: return "setarg0";
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
		case OP_BITNOT: return "bit_not";
		case OP_IMINUS: return "iminus";
		case OP_FADD: return "fadd";
		case OP_FSUB: return "fsub";
		case OP_FMUL: return "fmul";
		case OP_FDIV: return "fdiv";
		case OP_FMINUS: return "fminus";
		case OP_NOT: return "not";
		case OP_SYSCALL: return "syscall";
		case OP_INVOKE: return "invoke";
		case OP_INVOKEVIRTUAL: return "invokevirutal";
		case OP_RESERVE: return "reserve";
		case OP_RET: return "ret";
		case OP_RETVIRTUAL: return "retvirtual";
		case OP_JMP: return "jmp";
		case OP_JMPF: return "jmpf";
		case OP_JMPT: return "jmpt";
		case OP_ARR: return "arr";
		case OP_STR: return "str";
		case OP_LDLIB: return "ldlib";
		case OP_BEQ: return "beq";
		case OP_IEQ: return "ieq";
		case OP_FEQ: return "feq";
		case OP_CEQ: return "ceq";
		case OP_ARREQ: return "arreq";
		case OP_BNE: return "bne";
		case OP_INE: return "ine";
		case OP_FNE: return "fne";
		case OP_CNE: return "cne";
		case OP_LT: return "lt";
		case OP_GT: return "gt";
		case OP_LE: return "le";
		case OP_GE: return "ge";
		case OP_BAND: return "band";
		case OP_BOR: return "bor";
		case OP_GETSUB: return "getsub";
		case OP_SETSUB: return "setsub";
		case OP_LEN: return "len";
		case OP_CONS: return "cons";
		case OP_APPEND: return "append";
		case OP_UPVAL: return "upval";
		case OP_UPSTORE: return "upstore";
		case OP_CLASS: return "class";
		case OP_SETFIELD: return "setfield";
		case OP_GETFIELD: return "getfield";
		default: return "unknown";
	}
}

instruction_t* instruction_new(opcode_t op)
{
	instruction_t* ins = malloc(sizeof(*ins));
	ins->op = op;
	ins->v1 = NULL_VAL;
	ins->v2 = NULL_VAL;
	return ins;
}

void push_ins(vector_t* buffer, instruction_t* ins)
{
	vector_push(buffer, ins);
}

void insert(vector_t* buffer, opcode_t op)
{
	push_ins(buffer, instruction_new(op));
}

void insert_v1(vector_t* buffer, opcode_t op, val_t v1)
{
	instruction_t* ins = instruction_new(op);
	ins->v1 = v1;
	push_ins(buffer, ins);
}

void insert_v2(vector_t* buffer, opcode_t op, val_t v1, val_t v2)
{
	instruction_t* ins = instruction_new(op);
	ins->v1 = v1;
	ins->v2 = v2;
	push_ins(buffer, ins);
}

// Main functions

void emit_bool(vector_t* buffer, bool b)
{
	val_t val = BOOL_VAL(b);
	insert_v1(buffer, OP_PUSH, val);
}

void emit_int(vector_t* buffer, I64 v)
{
	val_t val = NUM_VAL(v);
	insert_v1(buffer, OP_PUSH, val);
}

void emit_float(vector_t* buffer, F64 f)
{
	val_t val = NUM_VAL(f);
	insert_v1(buffer, OP_PUSH, val);
}

void emit_char(vector_t* buffer, char c)
{
	val_t val = NUM_VAL(c);
	insert_v1(buffer, OP_PUSH, val);
}

void emit_string(vector_t* buffer, char* str)
{
	obj_t* obj = obj_string_new(str);
	val_t val = OBJ_VAL(obj);
	insert_v1(buffer, OP_PUSH, val);
}

void emit_pop(vector_t* buffer)
{
	insert(buffer, OP_POP);
}

void emit_op(vector_t* buffer, opcode_t op)
{
	insert(buffer, op);
}

opcode_t getOp(token_type_t tok, datatype_t dt)
{
	type_t type = dt.type;
	switch(tok)
	{
		case TOKEN_ADD:
		{
			if(type == DATA_INT) return OP_IADD;
			if(type == DATA_FLOAT) return OP_FADD;
			return -1;
		}
		case TOKEN_SUB:
		{
			if(type == DATA_INT) return OP_ISUB;
			if(type == DATA_FLOAT) return OP_FSUB;
			return -1;
		}
		case TOKEN_MUL:
		{
			if(type == DATA_INT) return OP_IMUL;
			if(type == DATA_FLOAT) return OP_FMUL;
			return -1;
		}
		case TOKEN_DIV:
		{
			if(type == DATA_INT) return OP_IDIV;
			if(type == DATA_FLOAT) return OP_FDIV;
			return -1;
		}
		case TOKEN_MOD: return OP_MOD;
		case TOKEN_BITLSHIFT: return OP_BITL;
		case TOKEN_BITRSHIFT: return OP_BITR;
		case TOKEN_BITAND: return OP_BITAND;
		case TOKEN_BITOR: return OP_BITOR;
		case TOKEN_BITXOR: return OP_BITXOR;
		case TOKEN_BITNOT: return OP_BITNOT;
		case TOKEN_EQUAL:
		{
			if(type == DATA_BOOL) return OP_BEQ;
			if(type == DATA_INT) return OP_IEQ;
			if(type == DATA_FLOAT) return OP_FEQ;
			if(type == DATA_CHAR) return OP_CEQ;
			return -1;
		}
		case TOKEN_NEQUAL:
		{
			if(type == DATA_BOOL) return OP_BNE;
			if(type == DATA_INT) return OP_INE;
			if(type == DATA_FLOAT) return OP_FNE;
			if(type == DATA_CHAR) return OP_CNE;
			return -1;
		}
		case TOKEN_LESS: return OP_LT;
		case TOKEN_GREATER: return OP_GT;
		case TOKEN_LEQUAL: return OP_LE;
		case TOKEN_GEQUAL: return OP_GE;
		case TOKEN_AND: return OP_BAND;
		case TOKEN_OR: return OP_BOR;
		default:
		{
			return -1;
		}
	}
}

bool emit_tok2op(vector_t* buffer, token_type_t tok, datatype_t type)
{
	opcode_t op = getOp(tok, type);
	emit_op(buffer, op);
	return op == -1 ? false : true;
}

void emit_syscall(vector_t* buffer, char* name, size_t args)
{
	val_t v1 = OBJ_VAL(obj_string_new(name));
	val_t v2 = NUM_VAL(args);
	insert_v2(buffer, OP_SYSCALL, v1, v2);
}

void emit_invoke(vector_t* buffer, size_t address, size_t args)
{
	val_t v1 = NUM_VAL(address);
	val_t v2 = NUM_VAL(args);
	insert_v2(buffer, OP_INVOKE, v1, v2);
}

void emit_return(vector_t* buffer)
{
	insert(buffer, OP_RET);
}

void emit_store(vector_t* buffer, int address, bool global)
{
	val_t val = NUM_VAL(address);
	insert_v1(buffer, global? OP_GSTORE : OP_STORE, val);
}

void emit_load(vector_t* buffer, int address, bool global)
{
	val_t val = NUM_VAL(address);
	insert_v1(buffer, global? OP_GLOAD : OP_LOAD, val);
}

void emit_load_upval(vector_t* buffer, int depth, int address)
{
	insert_v2(buffer, OP_UPVAL, NUM_VAL(depth), NUM_VAL(address));
}

void emit_store_upval(vector_t* buffer, int depth, int address)
{
	insert_v2(buffer, OP_UPSTORE, NUM_VAL(depth), NUM_VAL(address));
}

val_t* emit_jmp(vector_t* buffer, int address)
{
	val_t val = NUM_VAL(address);
	insert_v1(buffer, OP_JMP, val);
	return vector_top(buffer);
}

val_t* emit_jmpf(vector_t* buffer, int address)
{
	val_t val = NUM_VAL(address);
	insert_v1(buffer, OP_JMPF, val);
	return vector_top(buffer);
}

val_t* emit_jmpt(vector_t* buffer, int address)
{
	val_t val = NUM_VAL(address);
	insert_v1(buffer, OP_JMPT, val);
	return vector_top(buffer);
}

void emit_class_setfield(vector_t* buffer, int address)
{
	insert_v1(buffer, OP_SETFIELD, NUM_VAL(address));
}

void emit_class_getfield(vector_t* buffer, int address)
{
	insert_v1(buffer, OP_GETFIELD, NUM_VAL(address));
}

void emit_lib_load(vector_t* buffer, char* name)
{
	insert_v1(buffer, OP_LDLIB, OBJ_VAL(obj_string_new(name)));
}
