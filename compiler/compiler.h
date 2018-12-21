/**
 * Compiler
 * Copyright (C) 2017 Alexander Koch
 * Compiles an abstract syntax tree (AST) into bytecode instructions
 *
 * Scopes are managed, ASTs are optimized and converted to bytecode instructions.
 * An error is thrown by the compiler if the semantics are wrong.
 */

#ifndef compiler_h
#define compiler_h

#include <stdio.h>
#include <stdlib.h>
#include <core/mem.h>
#include <core/util.h>
#include <parser/parser.h>
#include <parser/types.h>
#include <vm/bytecode.h>
#include <compiler/scope.h>
#include <lib/native.h>

typedef struct compiler_t {
    context_t* context;
    parser_t* parser;
    scope_t* scope;
    hashmap_t* imports;
    vector_t* buffer;
    bool error;
    int depth;
} compiler_t;

vector_t* compile_buffer(const char* name, char* source);
vector_t* compile_file(const char* filename);

#endif
