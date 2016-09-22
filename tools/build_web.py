# Script to generate golem.js
# Make sure emcc is in path
# @author Alexander Koch
import os

flags = "-DNO_IR -DNO_MEMINFO -DNO_AST"
files = ["adt/bytebuffer.c",
		"adt/hashmap.c",
		"adt/list.c",
		"adt/vector.c",
		"compiler/compiler.c",
		"compiler/scope.c",
        "compiler/graphviz.c",
        "compiler/scope.c",
		"compiler/serializer.c",
		"core/util.c",
		"lexis/lexer.c",
		"lib/corelib.c",
		"lib/iolib.c",
		"lib/mathlib.c",
        "parser/ast.c",
        "parser/parser.c",
        "parser/types.c",
        "vm/bytecode.c",
        "vm/val.c",
        "vm/vm.c",
		"tools/web.c"]

objects = map(lambda x: x.split('/')[1][:-2]+".bc", files)
obj_list = ' '.join(objects)

for f, obj in zip(files, objects):
    print("{0} -> {1}".format(f, obj))
    os.system("emcc -Wall -O2 -I. " + flags + " " + f + " -o " + obj)

print("Generating golem.js ...")
exported_functions = "-s EXPORTED_FUNCTIONS=\"['_golem_interpret']\""
emccflags = exported_functions + " -s ALLOW_MEMORY_GROWTH=1 -s NO_FILESYSTEM=1 -s NO_EXIT_RUNTIME=1 --memory-init-file 0 -s NO_BROWSER=1"
os.system("emcc -O2 " + obj_list + " -o golem.js " + emccflags)
os.system("rm *.bc")
