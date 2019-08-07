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

all_inputs = " ".join(to_compile)
c = os.system("clang -c -std=c99 -Wall -Werror %s -g -DVK_USE_PLATFORM_XCB_KHR" % all_inputs)

if len(sys.argv) > 1 and sys.argv[1] == "run" and c == 0:
    os.system("./zgae")
