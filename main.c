/** Copryright (c) Alexander Koch 2015 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define USE_MEM_IMPLEMENTATION
#include <core/mem.h>
#include <vm/vm.h>
#include <ast.h>
#include <parser.h>

#include <adt/hashmap.h>

void run_repl(vm_t* vm)
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
        vm_run_buffer(vm, buf);
    }
}

int main(int argc, char** argv)
{
    vm_t vm;
    vm_init(&vm);

    if(argc == 1)
    {
        run_repl(&vm);
    }
    else if(argc == 2)
    {
        vm_run_file(&vm, argv[1]);
    }
    else
    {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, " nemesis        (Interactive mode)\n");
        fprintf(stderr, " nemesis <file> (Run a file)\n");
        vm_free(&vm);
        return -1;
    }

    vm_free(&vm);
    mem_leak_check();
    return 0;
}
