#include "scope.h"

// Scope.new()
// Scopes store the local state of the compiler.
// These are mainly used for functions and
// store each symbol created during the compilation.
scope_t* scope_new() {
    scope_t* scope = malloc(sizeof(*scope));
    scope->symbols = hashmap_new();
    scope->classes = hashmap_new();
    scope->super = 0;
    scope->subscopes = list_new();
    scope->address = 0;
    scope->node = 0;
    scope->flag = 0;
    scope->virtual = false;
    return scope;
}

int freeSymbols(void* arg, void* val) {
    free(arg);
    return 0;
}

// Scope.free()
void scope_free(scope_t* scope) {
    // Free the symbols
    hashmap_foreach(scope->symbols, freeSymbols, 0);
    hashmap_free(scope->symbols);
    hashmap_free(scope->classes);

    // Free subscopes
    list_iterator_t* liter = list_iterator_create(scope->subscopes);
    while(!list_iterator_end(liter)) {
        scope_t* subscope = list_iterator_next(liter);
        scope_free(subscope);
    }
    list_iterator_free(liter);
    list_free(scope->subscopes);

    // Free the actual scope
    free(scope);
    scope = 0;
}

bool scope_requests(scope_t* scope, annotation_t ann) {
    return (scope->flag & ann) == ann;
}

void scope_unflag(scope_t* scope) {
    scope->flag = 0;
}
