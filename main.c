#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define USE_MEM_IMPLEMENTATION
#include <core/mem.h>
#include <vm/vm.h>
#include <ast.h>
#include <parser.h>

void run_interactive()
{
    vm_t vm;
    char buf[1024];
    while(1)
    {
        fprintf(stdout, ">>> ");
        fgets(buf, sizeof(buf), stdin);
        if(!strncmp(buf, "quit", 4))
        {
            return;
        }
        vm_run_buffer(&vm, buf);
    }
}

int main(int argc, char** argv)
{
    if(argc == 1)
    {
        run_interactive();
    }
    else if(argc == 2)
    {
        vm_t vm;
        vm_run_file(&vm, argv[1]);
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
