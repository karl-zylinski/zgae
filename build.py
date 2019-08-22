#!/usr/bin/env python3

import os
import sys

all_files = os.listdir(".")
to_compile = []

for f in all_files:
    if not os.path.isfile(f):
        continue

    if f.endswith(".c"):
        to_compile.append(f)

if not os.path.isdir("build"):
    os.mkdir("build")

built_objects = []

extra_flags = [
    "-DENABLE_MEMORY_TRACING",
    "-DENABLE_SLOW_DEBUG_CHECKS"
]

if "stop_on_first_error" in sys.argv:
    extra_flags.append("-Wfatal-errors")

for in_filename in to_compile:
    object_filename = in_filename[0:-2] + ".o"
    out_filename = "build/" + object_filename

    extra_flags_str = " ".join(extra_flags)
    compile_error = os.system("clang -c -std=gnu11 -Wall -Wextra -Werror %s -o %s -g -include global_include.h -DVK_USE_PLATFORM_XCB_KHR %s -Wno-unused-function" % (in_filename, out_filename, extra_flags_str))

    if compile_error != 0:
        exit("\nbuild.py exited: compilation error")

    built_objects.append(out_filename)


linker_input_str = " ".join(built_objects)
linker_error = os.system("clang %s -rdynamic -o zgae -lrt -lm -lxcb -lvulkan" % linker_input_str)

if linker_error != 0:
    exit("\nbuild.py exited: linker error")


os.system("glslc -fshader-stage=frag shader_default_fragment.glsl -o shader_default_fragment.spv")
os.system("glslc -fshader-stage=vertex shader_default_vertex.glsl -o shader_default_vertex.spv")

if "run" in sys.argv:
    os.system("./zgae")


