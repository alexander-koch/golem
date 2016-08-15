/**
 * Copyright (c) 2015-2016
 * @author Alexander Koch
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define USE_MEM_IMPLEMENTATION
#include <core/mem.h>
#include <core/util.h>
#include <parser/ast.h>
#include <parser/parser.h>
#include <vm/vm.h>
#include <compiler/compiler.h>
#include <compiler/serializer.h>
#include <compiler/graphviz.h>
#include <compiler/doc.h>

void print_info() {
    printf("Golem compiler\n");
    printf("Copyright (c) Alexander Koch 2016\n\n");
    printf("Usage:\n");
    printf("  golem <file>       (Run a file)\n");
    printf("  golem -r <file>    (Run a *.gvm file)\n");
    printf("  golem -c <file>    (Convert to bytecode file *.gvm)\n");
    printf("  golem --ast <file> (Convert generated AST to graph *.dot)\n");
    printf("  golem --doc <file> (Create an HTML documentation)\n\n");
}

int main(int argc, char** argv) {
    seed_prng(time(0));
    vm_t vm = {0};

    if(argc == 2) {
        // Generate and execute bytecode (Interpreter)
        vector_t* buffer = compile_file(argv[1]);
        if(buffer) {
            vm_run_args(&vm, buffer, argc, argv);
            bytecode_buffer_free(buffer);
        }
    } else if(argc == 3) {
        if(!strcmp(argv[1], "-c")) {
            // Compile to bytecode
            vector_t* buffer = compile_file(argv[2]);
            if(buffer) {
                // Write to file
                char* out = replaceExt(argv[2], ".gvm", 4);
                serialize(out, buffer);
                printf("Wrote bytecode to file '%s'\n", out);
                free(out);
                bytecode_buffer_free(buffer);
            }
        } else if(!strcmp(argv[1], "-r")) {
            // Run compiled bytecode file
            vector_t* buffer = vector_new();
            if(deserialize(argv[2], &buffer)) {
                vm_run_args(&vm, buffer, argc, argv);
            }
            bytecode_buffer_free(buffer);
        } else if(!strcmp(argv[1], "--ast")) {
            // Generate ast.dot graphviz file
            char* path = argv[2];
            char* source = readFile(path);
            if(!source) {
                printf("File '%s' does not exist\n", path);
                return 1;
            }

            parser_t parser;
            parser_init(&parser, path);
            ast_t* root = parser_run(&parser, source);
            if(root) {
                graphviz_build(root);
            }
            ast_free(parser.top);
            parser_free(&parser);
            free(source);
        } else if(!strcmp(argv[1], "--doc")) {
            // Generate HTML-doc
            doc_generate(argv[2]);
        } else {
            printf("Flag: '%s' is invalid\n\n", argv[1]);
            return 1;
        }
    } else {
        print_info();
    }

#ifndef NO_MEMINFO
    mem_leak_check();
#endif
    return 0;
}
