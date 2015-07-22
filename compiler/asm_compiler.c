#include "asm_compiler.h"

void asm_write_prologue(FILE* fp, int stack_size)
{
	fprintf(fp, "extern printf\nsection .data\n\tsize equ %d\n\tfmt db \"%%d\", 0\n\n", stack_size);
	fprintf(fp, "section .text\nglobal WinMain\nWinMain:\n");
	fprintf(fp, "\t; prologue\n\tmov [rsp+8],rcx\n\tpush r14\n\tpush r13\n");
	fprintf(fp, "\tsub rsp, size\n\tlea r13,[rsp]\n\n");
}

void asm_write_epilogue(FILE* fp)
{
	fprintf(fp, "\t; epilogue\n\tlea rsp,[r13]\n\tadd rsp,size\n");
	fprintf(fp, "\tpop r13\n\tpop r14\n\tret\n");
}

void translate(FILE* fp, instruction_t* instr)
{
	switch(instr->op)
	{
		case OP_PUSH_INT:
		{
			fprintf(fp, "\tmov rax, %li\n", (long int)value_int(instr->v1));
			fprintf(fp, "\tpush rax\n");
			break;
		}
		case OP_STORE_FIELD:
		{
			fprintf(fp, "\tpop rax\n");
			fprintf(fp, "\t; store to %s\n", value_string(instr->v1));
			break;
		}
		case OP_INVOKE:
		{
			if(!strcmp(value_string(instr->v1), "println"))
			{
				fprintf(fp, "\tpop rax\n");
				fprintf(fp, "\tmov rcx, fmt\n");
				fprintf(fp, "\tmov rdx, rax\n");
				fprintf(fp, "\tcall printf\n");
			}
			break;
		}
		default: break;
	}
}

int asm_write(compiler_t* compiler, const char* filename)
{
	FILE* fp = fopen(filename, "wb");
	if(!fp) return 1;
	asm_write_prologue(fp, 256);

	fprintf(fp, "\t; main code\n");
	list_iterator_t* iter = list_iterator_create(compiler->buffer);
	while(!list_iterator_end(iter))
	{
		instruction_t* instr = (instruction_t*)list_iterator_next(iter);
		translate(fp, instr);
	}
	list_iterator_free(iter);
	fprintf(fp, "\n");

	asm_write_epilogue(fp);
	fclose(fp);
	return 0;
}
