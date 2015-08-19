/** Copyright (c) 2015 Alexander Koch All Rights Reserved. **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define USE_MEM_IMPLEMENTATION
#include <core/mem.h>
#include <parser/ast.h>
#include <parser/parser.h>
#include <vm/vm.h>
#include <compiler/compiler.h>
#include <compiler/bytecode_writer.h>

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
        vector_t* buffer = compile_buffer(compiler, buf);
        if(buffer)
        {
            vm_execute(vm, buffer);
        }
        compiler_clear(compiler);
    }
}

int main(int argc, char** argv)
{
    compiler_t compiler;
    compiler_init(&compiler);

    if(argc == 1)
    {
        // Read-eval-print-loop
        vm_t* vm = vm_new();
        run_repl(vm, &compiler);
        vm_free(vm);
    }
    else if(argc == 2)
    {
        // Generate and execute bytecode
        vm_t* vm = vm_new();

        vector_t* buffer = compile_file(&compiler, argv[1]);
        if(buffer)
        {
            vm_execute(vm, buffer);
            compiler_clear(&compiler);
        }
        else
        {
            fprintf(stdout, "Could not compile file '%s'\n\n", argv[1]);
        }

        vm_free(vm);
    }
    else if(argc == 3)
    {
        // Compile to assembly
        if(!strcmp(argv[1], "-c"))
        {
            vector_t* buffer = compile_file(&compiler, argv[2]);
            if(buffer)
            {
                write_bytecode("out.app", compiler.buffer);
                compiler_clear(&compiler);
                fprintf(stdout, "Wrote assembly code to file 'out.app'\n");
            }
            else
            {
                fprintf(stdout, "Could not compile file '%s'\n\n", argv[2]);
            }
        }
        else if(!strcmp(argv[1], "-r"))
        {
            vm_t* vm = vm_new();
            vector_t* buffer = vector_new();
            bool ok = read_bytecode(argv[2], &buffer);
            compiler.buffer = buffer;
            if(ok)
            {
                vm_execute(vm, buffer);
            }
            compiler_clear(&compiler);
            vm_free(vm);
        }
        else
        {
            fprintf(stderr, "Flag: '%s' is invalid\n\n", argv[1]);
        }
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
