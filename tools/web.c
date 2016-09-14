#include <compiler/compiler.h>
#include <core/util.h>
#include <vm/vm.h>

int golem_interpret(const char* module, const char* source) {
    seed_prng(time(0));
	vm_t vm;
	memset(&vm, 0, sizeof(vm_t));
	vector_t* buffer = compile_buffer(source, module);
	if(buffer) {
		vm_run(&vm, buffer);
		bytecode_buffer_free(buffer);
	}
	return 0;
}
