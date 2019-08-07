#!/usr/bin/env python3

import os
import sys

from os.path import isfile

all_files = os.listdir(".")
to_compile = []

for f in all_files:
    if not isfile(f):
        continue

    if f.endswith(".c"):
        to_compile.append(f)

if not os.path.isdir("build"):
    os.mkdir("build")

built_objects = []

for in_filename in to_compile:
    object_filename = in_filename[0:-2] + ".o"
    out_filename = "build/" + object_filename
    compile_error = os.system("clang -c -std=c99 -Wall -Werror %s -o %s -g -DVK_USE_PLATFORM_XCB_KHR" % (in_filename, out_filename))

    if compile_error != 0:
        exit("\nbuild.py exited: compilation error")

    built_objects.append(out_filename)


linker_input_str = " ".join(built_objects)
linker_error = os.system("clang %s -o zgae -lxcb -lvulkan" % linker_input_str)

if linker_error != 0:
    exit("\nbuild.py exited: linker error")

if len(sys.argv) > 1 and sys.argv[1] == "run":
    os.system("./zgae")
