#ifndef eval_h
#define eval_h

#include <vm/eval.h>

// shunting yard implementation
// queue + stack

value_t* eval_expression(ast_t* node);

#endif
