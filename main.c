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
#include <compiler/serializer.h>
#include <compiler/graphviz.h>
#include <compiler/doc.h>

void run_repl(vm_t* vm, compiler_t* compiler)
{
    char buf[1024];
    while(1)
    {
        printf(">> ");
        fgets(buf, sizeof(buf), stdin);
        if(!strncmp(buf, "quit", 4))
        {
            return;
        }
        vector_t* buffer = compile_buffer(compiler, buf, "Unnamed");
        if(buffer)
        {
            vm_run(vm, buffer);
        }
        compiler_clear(compiler);
    }
}

void print_info()
{
    printf("Golem compiler\n");
    printf("Copyright (c) Alexander Koch 2015\nAll Rights Reserved.\n\n");
    printf("Usage:\n");
    printf("  golem <file>       (Run a file)\n");
    printf("  golem -r <file>    (Run a *.gvm file)\n");
    printf("  golem -c <file>    (Convert to bytecode file (*.gvm))\n");
    printf("  golem --ast <file> (Convert generated AST to graph)\n\n");
}

int main(int argc, char** argv)
{
    compiler_t compiler;
    compiler_init(&compiler);

    /*if(argc == 1)
    {
        // Read-eval-print-loop
        printf("Golem compiler - REPL\n");
        printf("Copyright (c) Alexander Koch 2015 All Rights Reserved.\n\n");

        vm_t* vm = vm_new();
        run_repl(vm, &compiler);
        vm_free(vm);
    }*/
    if(argc == 2)
    {
        // Generate and execute bytecode (Interpreter)
        vm_t* vm = vm_new();
        vector_t* buffer = compile_file(&compiler, argv[1]);
        if(buffer)
        {
            vm_run(vm, buffer);
            compiler_clear(&compiler);
        }
        vm_free(vm);
    }
    else if(argc == 3)
    {
        if(!strcmp(argv[1], "-c"))
        {
            vector_t* buffer = compile_file(&compiler, argv[2]);
            if(buffer)
            {
                // Write to file
                char* out = replaceExt(argv[2], ".gvm", 4);
                serialize(out, compiler.buffer);

                // Clear the compiler
                compiler_clear(&compiler);
                printf("Wrote bytecode to file '%s'\n", out);
                free(out);
            }
        }
        else if(!strcmp(argv[1], "-r"))
        {
            // Run compiled bytecode file
            vm_t* vm = vm_new();
            vector_t* buffer = vector_new();
            bool ok = deserialize(argv[2], &buffer);
            compiler.buffer = buffer;
            if(ok)
            {
                vm_run(vm, buffer);
            }
            compiler_clear(&compiler);
            vm_free(vm);
        }
        else if(!strcmp(argv[1], "--ast"))
        {
            char* filename = argv[2];
            size_t len = 0;
        	char* source = readFile(filename, &len);

            parser_t parser;
            parser_init(&parser, filename);
            ast_t* root = parser_run(&parser, source);
            if(root)
            {
                graphviz_build(root);
            }
            ast_free(parser.top);
            parser_free(&parser);
            free(source);
        }
        else if(!strcmp(argv[1], "--doc"))
        {
            doc_generate(argv[2]);
        }
        else
        {
            printf("Flag: '%s' is invalid\n\n", argv[1]);
            return 1;
        }
    }
    else
    {
        print_info();
        return 0;
    }

#ifndef NO_MEMINFO
    mem_leak_check();
#endif
    return 0;
}
