/** Copryright (c) Alexander Koch 2015 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define USE_MEM_IMPLEMENTATION
#include <core/mem.h>
#include <parser/ast.h>
#include <parser/parser.h>
#include <vm/compiler.h>

void run_repl(compiler_t* vm)
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
        list_t* buffer = compile_buffer(vm, buf);
        buffer_free(buffer);
    }
}

int main(int argc, char** argv)
{
    compiler_t compiler;

    if(argc == 1)
    {
        run_repl(&compiler);
    }
    else if(argc == 2)
    {
        list_t* buffer = compile_file(&compiler, argv[1]);
        buffer_free(buffer);
    }
    else
    {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, " nemesis        (Interactive mode)\n");
        fprintf(stderr, " nemesis <file> (Run a file)\n");
        return -1;
    }

    mem_leak_check();
    return 0;
}
