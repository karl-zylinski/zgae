#!/usr/bin/env python3

import os
import sys
from datetime import datetime

all_files = os.listdir(".")
to_compile = []
shaders = []

entry_files = ["main_linux_xlib_vulkan.cpp", "tests.cpp"]

for f in all_files:
    if not os.path.isfile(f):
        continue

    if f.endswith(".cpp") and f not in entry_files:
        to_compile.append(f)

    if f.endswith(".glsl"):
        shaders.append(f)

output = "zgae"

if "tests" in sys.argv:
    to_compile.append("tests.cpp")
    output = "tests"
else:
    to_compile.append("main_linux_xlib_vulkan.cpp")

if not os.path.isdir("build"):
    os.mkdir("build")

built_objects = []

extra_flags = [
    "-DENABLE_MEMORY_TRACING",
    "-DENABLE_SLOW_DEBUG_CHECKS"
]

if "nounusedwarning" in sys.argv:
    extra_flags.append("-Wno-unused-variable -Wno-unused-parameter")

compiler = os.environ["CC"] if ("CC" in os.environ) else "clang++"

static_analysis = "static_analysis" in sys.argv
stop_on_error = not static_analysis

if "stop_on_first_error" in sys.argv:
    extra_flags.append("-Wfatal-errors")

if not static_analysis:
    extra_flags.append("-Werror")

compile_start = datetime.now()
for in_filename in to_compile:
    object_filename = in_filename[0:-4] + ".o"
    out_filename = "build/" + object_filename

    extra_flags_str = " ".join(extra_flags)
    compile_error = os.WEXITSTATUS(os.system("%s -c -std=c++11 -Wall -Wextra %s -o %s -g -include global_include.h -Wno-writable-strings -DVK_USE_PLATFORM_XLIB_KHR %s -Wno-unused-function -Wno-attributes" % (compiler, in_filename, out_filename, extra_flags_str)))

    if stop_on_error and compile_error != 0:
        exit("\nbuild.py exited: compilation error")

    built_objects.append(out_filename)
compile_end = datetime.now()
compile_dt = compile_end - compile_start

print("Compile dt: %f s" % (compile_dt.seconds + compile_dt.microseconds/1000000.0))

if static_analysis:
    exit("static analysis done")

link_start = datetime.now()
linker_input_str = " ".join(built_objects)
linker_error = os.WEXITSTATUS(os.system("%s %s -rdynamic -o %s -lrt -lm -lX11 -lvulkan" % (compiler, linker_input_str, output)))
link_end = datetime.now()
link_dt = link_end - link_start
total_dt = link_end - compile_start

print("Link dt: %f s" % (link_dt.seconds + link_dt.microseconds/1000000.0))
print("Total dt: %f s" % (total_dt.seconds + total_dt.microseconds/1000000.0))

if stop_on_error and linker_error != 0:
    exit("\nbuild.py exited: linker error")

for s in shaders:
    t = "frag" if ("frag" in s) else "vertex"
    name_in = s
    name_out = s[0:-5] + ".spv"
    shader_error = os.WEXITSTATUS(os.system("glslc -fshader-stage=%s %s -o %s" % (t, name_in, name_out)))

    if shader_error != 0:
        exit("\n%s didn't compile" % name_in)


