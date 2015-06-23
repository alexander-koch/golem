/** Copryright (c) Alexander Koch 2015 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define USE_MEM_IMPLEMENTATION
#include <core/mem.h>
#include <parser/ast.h>
#include <parser/parser.h>
#include <compiler/compiler.h>
#include <vm/vm.h>

void run_repl(vm_t* vm, compiler_t* compiler)
{
    char buf[1024];
    while(1)
    {
        fprintf(stdout, ">> ");
        fgets(buf, sizeof(buf), stdin);
        if(!strncmp(buf, "quit", 4))
        {
            return;
        }
        list_t* buffer = compile_buffer(compiler, buf);
        vm_execute(vm, buffer);
        compiler_clear(compiler);
    }
}

int main(int argc, char** argv)
{
    compiler_t compiler;
    compiler.buffer = 0;
    compiler.filename = 0;

    if(argc == 1)
    {
        vm_t* vm = vm_new();
        run_repl(vm, &compiler);
        vm_free(vm);
    }
    else if(argc == 2)
    {
        vm_t* vm = vm_new();
        list_t* buffer = compile_file(&compiler, argv[1]);
        if(buffer)
        {
            vm_execute(vm, buffer);
            compiler_clear(&compiler);
        }

        vm_free(vm);
    }
    else
    {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, " golem        (Interactive mode)\n");
        fprintf(stderr, " golem <file> (Run a file)\n");
        return -1;
    }

    mem_leak_check();
    return 0;
}
