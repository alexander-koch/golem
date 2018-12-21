// Copyright (C) 2017 Alexander Koch
#include <stdio.h>
#include <compiler/serializer.h>
#include <vm/bytecode.h>

int main(int argc, char **argv) {
	if(argc != 2) {
		printf("Usage: golem-ir file\n");
		return 0;
	}

	vector_t* buffer = vector_new();
	if(deserialize(argv[1], &buffer)) {
		for(size_t i = 0; i < vector_size(buffer); i++) {
			instruction_t* instr = vector_get(buffer, i);

			// Print
			printf("%.2u: %s", (unsigned int)i, op2str(instr->op));
			if(instr->v1 != NULL_VAL) {
				printf(", ");
				val_print(instr->v1);
			}
			if(instr->v2 != NULL_VAL) {
				printf(", ");
				val_print(instr->v2);
			}
			putchar('\n');
		}
	} else {
		printf("Could not read file '%s'\n", argv[1]);
	}

	bytecode_buffer_free(buffer);
	return 0;
}
