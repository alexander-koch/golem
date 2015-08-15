#!/usr/bin/env python

import glob
import fnmatch
import itertools
import os
import re

TODO_PATTERN = re.compile(r'\s*// TODO:')
DOC_PATTERN = re.compile(r'\s*//')
FIXME_PATTERN = re.compile(r'\s*// FIXME:')

num_files = 0
num_docs = 0
num_code = 0
num_empty = 0
num_todos = 0
num_semicolons = 0
num_fixmes = 0

files = itertools.chain(glob.iglob("adt/*.[ch]"),
                        glob.iglob("compiler/*.[ch]"),
                        glob.iglob("core/*.[ch]"),
                        glob.iglob("lexis/*.[ch]"),
                        glob.iglob("parser/*.[ch]"),
                        glob.iglob("vm/*.[ch]"),
                        glob.iglob("*.[ch]"))

for source_path in files:
    num_files += 1
    with open(source_path, "r") as input:
        for line in input:
            num_semicolons += line.count(';')
            num_code += 1

            match = TODO_PATTERN.match(line)
            if match:
                num_todos += 1
                num_docs += 1
                continue

            match = FIXME_PATTERN.match(line)
            if match:
                num_fixmes += 1
                num_docs += 1
                continue

            match = DOC_PATTERN.match(line)
            if match:
                num_docs += 1
                continue

            if (line.strip() == ""):
                num_empty += 1

lloc = num_code - num_docs - num_empty
llocp = float(lloc) / float(num_code)
mcomm = float(num_docs) / float(lloc)

print("Quality indicators:")
print("  files           " + str(num_files))
print("  semicolons      " + str(num_semicolons))
print("  TODOs           " + str(num_todos))
print("  FIXMEs          " + str(num_fixmes))
print("  comment lines   " + str(num_docs))
print("  empty lines     " + str(num_empty))
print("  LOC             " + str(num_code))
print("\n")
print("Metrics:")
print("  LLOC            " + str(lloc))
print("  LLOC%           " + str(llocp))
print("  MCOMM%          " + str(mcomm) + " <= Expected 20%")
