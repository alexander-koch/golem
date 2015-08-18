#include "bytecode_writer.h"

// Overall structure

// APP<code count>
// |Entry0  |
// |instr   |
// |args    |
// |values  |
// |End		|
// | ...    |
// Entries2, 3, ..

// Writes one value_t to a file
void write_value(FILE* fp, value_t* val)
{
	fwrite((const void*)&val->type, sizeof(value_type_t), 1, fp);
	switch(val->type)
	{
		case VALUE_BOOL:
		case VALUE_INT:
		{
			fwrite((const void*)&val->v.i, sizeof(val->v.i), 1, fp);
			break;
		}
		case VALUE_FLOAT:
		{
			fwrite((const void*)&val->v.f, sizeof(val->v.f), 1, fp);
			break;
		}
		case VALUE_STRING:
		{
			// Write the length of the string first, to tell the reader the size
			uint32_t len = strlen(value_string(val));
			fwrite((const void*)&len, sizeof(uint32_t), 1, fp);
			fwrite((char*)val->v.o, sizeof(char), len, fp);
			break;
		}
		case VALUE_CHAR:
		{
			fwrite((const void*)&val->v.c, sizeof(val->v.c), 1, fp);
			break;
		}
		default: break;
	}
}

// Read one value of type value_t, allocates memory for it
value_t* read_value(FILE* fp)
{
	// Read basic struct, zero out
	value_type_t type;
	fread(&type, sizeof(value_type_t), 1, fp);
	value_t* val = value_new_null();
	val->type = type;

	// Based on read type, decide what to read next
	switch(val->type)
	{
		case VALUE_BOOL:
		case VALUE_INT:
		{
			fread(&val->v.i, sizeof(val->v.i), 1, fp);
			break;
		}
		case VALUE_FLOAT:
		{
			fread(&val->v.f, sizeof(val->v.f), 1, fp);
			break;
		}
		case VALUE_STRING:
		{
			uint32_t len = 0;
			fread(&len, sizeof(uint32_t), 1, fp);
			len++;

			val->v.o = malloc(sizeof(char)*len);
			fgets((char*)val->v.o, sizeof(char)*len, fp);
			break;
		}
		case VALUE_CHAR:
		{
			fread(&val->v.c, sizeof(val->v.c), 1, fp);
			break;
		}
		default: break;
	}

	return val;
}

void write_instruction(FILE* fp, instruction_t* ins)
{
	uint8_t opcode = ins->op;
	fwrite((const void*)&opcode, sizeof(uint8_t), 1, fp);

	uint8_t args = 0;
	if(ins->v1) args++;
	if(ins->v2) args++;
	fwrite((const void*)&args, sizeof(uint8_t), 1, fp);

	if(args > 0) write_value(fp, ins->v1);
	if(args > 1) write_value(fp, ins->v2);
}

bool write_bytecode(const char* filename, vector_t* instructions)
{
	FILE* fp = fopen(filename, "wb");
	if(!fp) return false;

	// Write magic number + amount of instructions to fetch
	uint32_t magic = 0x1337;
	uint32_t codes = vector_size(instructions);
	fwrite((const void*)&magic, sizeof(uint32_t), 1, fp);
	fwrite((const void*)&codes, sizeof(uint32_t), 1, fp);

	for(size_t i = 0; i < vector_size(instructions); i++)
	{
		instruction_t* ins = vector_get(instructions, i);
		write_instruction(fp, ins);
	}

	fclose(fp);
	return true;
}

bool read_bytecode(const char* filename, vector_t** out)
{
	FILE* fp = fopen(filename, "rb");
	if(!fp) return false;

	// Read the magic number + the code size
	uint32_t magic, codes;
	fread(&magic, sizeof(uint32_t), 1, fp);
	fread(&codes, sizeof(uint32_t), 1, fp);

	// Check if file is valid
	if(magic != 0x1337)
	{
		fclose(fp);
		return false;
	}

	// Fetch + process the codes
	for(int i = 0; i < codes; i++)
	{
		instruction_t* ins = malloc(sizeof(*ins));
		ins->v1 = 0;
		ins->v2 = 0;

		// First get the header
		uint8_t op, args;
		fread(&op, sizeof(uint8_t), 1, fp);
		fread(&args, sizeof(uint8_t), 1, fp);
		ins->op = op;

		// Read the values
		if(args > 0) ins->v1 = read_value(fp);
		if(args > 1) ins->v2 = read_value(fp);
		vector_push((*out), ins);
	}

	// Read success!
	fclose(fp);
	return true;
}
