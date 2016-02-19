/**
 * doc.h
 * @author Alexander Koch
 * @desc HTML documentation compiler.
 * Creates an HTML file with the source code,
 * the lexical tokens and the corresponding bytecode.
 */

#ifndef doc_h
#define doc_h

#include <compiler/compiler.h>
#include <compiler/graphviz.h>
#include <vm/bytecode.h>
#include <adt/vector.h>

void doc_generate(const char* filename);

#endif
