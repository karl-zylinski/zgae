#!/usr/bin/env python3

import os
import sys

all_files = os.listdir(".")
to_compile = []

entry_files = ["linux_main.cpp", "tests.cpp"]

for f in all_files:
    if not os.path.isfile(f):
        continue

    if f.endswith(".cpp") and f not in entry_files:
        to_compile.append(f)

output = "zgae"

if "tests" in sys.argv:
    to_compile.append("tests.cpp")
    output = "tests"
else:
    to_compile.append("linux_main.cpp")

if not os.path.isdir("build"):
    os.mkdir("build")

built_objects = []

extra_flags = [
    "-DENABLE_MEMORY_TRACING",
    "-DENABLE_SLOW_DEBUG_CHECKS"
]

compiler = os.environ["CC"] if ("CC" in os.environ) else "clang++"

static_analysis = "static_analysis" in sys.argv
stop_on_error = not static_analysis

if "stop_on_first_error" in sys.argv:
    extra_flags.append("-Wfatal-errors")

if not static_analysis:
    extra_flags.append("-Werror")

for in_filename in to_compile:
    object_filename = in_filename[0:-4] + ".o"
    out_filename = "build/" + object_filename

    extra_flags_str = " ".join(extra_flags)
    compile_error = os.system("%s -c -std=c++11 -Wall -Wextra %s -o %s -g -include global_include.h -Wno-writable-strings -DVK_USE_PLATFORM_XCB_KHR %s -Wno-unused-function -Wno-attributes" % (compiler, in_filename, out_filename, extra_flags_str))

    if stop_on_error and compile_error != 0:
        exit("\nbuild.py exited: compilation error")

    built_objects.append(out_filename)


if static_analysis:
    exit("static analysis done")

linker_input_str = " ".join(built_objects)
linker_error = os.system("%s %s -rdynamic -o %s -lrt -lm -lxcb -lvulkan" % (compiler, linker_input_str, output))

if stop_on_error and linker_error != 0:
    exit("\nbuild.py exited: linker error")

os.system("glslc -fshader-stage=frag shader_default_fragment.glsl -o shader_default_fragment.spv")
os.system("glslc -fshader-stage=vertex shader_default_vertex.glsl -o shader_default_vertex.spv")



