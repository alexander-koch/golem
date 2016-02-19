#include "serializer.h"

bool serialize_value(FILE* fp, val_t val) {
	uint8_t tag;
	if(IS_NUM(val)) {
		tag = TAG_NUM;
	}
	else if(IS_BOOL(val)) {
		tag = TAG_BOOL;
	}
	else if(IS_STRING(val)) {
		tag = TAG_STR;
	}
	else {
		return false;
	}

	fwrite((const void*)&tag, sizeof(uint8_t), 1, fp);
	if(tag != TAG_STR) {
		fwrite((val_t*)&val, sizeof(val_t), 1, fp);
	}
	else {
		char* str = AS_STRING(val);
		uint32_t len = strlen(str);

		fwrite((const void*)&len, sizeof(uint32_t), 1, fp);
		fwrite((char*)str, sizeof(char), len, fp);
	}

	return true;
}

bool serialize_instruction(FILE* fp, instruction_t* ins) {
	bool valid = true;
	uint8_t opcode = ins->op;
	fwrite((const void*)&opcode, sizeof(uint8_t), 1, fp);

	uint8_t args = 0;
	if(ins->v1 != NULL_VAL) args++;
	if(ins->v2 != NULL_VAL) args++;
	fwrite((const void*)&args, sizeof(uint8_t), 1, fp);

	if(args > 0) {
		valid &= serialize_value(fp, ins->v1);
	}
	if(args > 1) {
		valid &= serialize_value(fp, ins->v2);
	}

	return valid;
}

bool serialize(const char* filename, vector_t* buffer) {
	FILE* fp = fopen(filename, "wb");
	if(!fp) return false;

	// Write magic number + amount of instructions to fetch
	uint32_t magic = 0xACCE55;
	uint32_t codes = vector_size(buffer);
	fwrite((const void*)&magic, sizeof(uint32_t), 1, fp);
	fwrite((const void*)&codes, sizeof(uint32_t), 1, fp);

	bool valid = true;
	for(size_t i = 0; i < vector_size(buffer); i++) {
		instruction_t* ins = vector_get(buffer, i);
		valid &= serialize_instruction(fp, ins);
	}

	fclose(fp);
	return valid;
}

val_t deserialize_value(FILE* fp) {
	val_t ret = NULL_VAL;

	// Read the tag
	uint8_t tag = 0;
	fread(&tag, sizeof(uint8_t), 1, fp);

	// If not string, read directly
	if(tag != TAG_STR) {
		fread(&ret, sizeof(val_t), 1, fp);
	}
	// Otherwise, read the length, then the string data
	else {
		uint32_t len = 0;
		fread(&len, sizeof(uint32_t), 1, fp);
		if(len <= 0) return ret;

		char* str = malloc(sizeof(char) * (len+1));
		fread((char*)str, sizeof(char), len, fp);
		str[len] = '\0';
		ret = STRING_NOCOPY_VAL(str);
	}
	return ret;
}

bool deserialize(const char* filename, vector_t** out) {
	FILE* fp = fopen(filename, "rb");
	if(!fp) return false;

	// Read the magic number + the code size
	uint32_t magic, codes;
	fread(&magic, sizeof(uint32_t), 1, fp);
	fread(&codes, sizeof(uint32_t), 1, fp);

	if(magic != 0xACCE55) {
		fclose(fp);
		return false;
	}

	// Read the instructions
	for(int i = 0; i < codes; i++) {
		instruction_t* ins = malloc(sizeof(*ins));
		ins->v1 = NULL_VAL;
		ins->v2 = NULL_VAL;

		// First get the header
		uint8_t op, args;
		fread(&op, sizeof(uint8_t), 1, fp);
		fread(&args, sizeof(uint8_t), 1, fp);
		ins->op = op;

		// Read the values
		if(args > 0) ins->v1 = deserialize_value(fp);
		if(args > 1) ins->v2 = deserialize_value(fp);
		vector_push((*out), ins);
	}

	fclose(fp);
	return true;
}
