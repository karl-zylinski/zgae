#!/usr/bin/env python3

import os
import sys

all_files = os.listdir(".")
to_optimize = []

for f in all_files:
    if not os.path.isfile(f):
        continue

    if f.endswith(".c") or f.endswith(".h"):
        to_optimize.append(f)

for f in to_optimize:
    org_fp = open(f, "r")
    org_file = org_fp.read()
    org_fp.seek(0, 0)
    org_filelines = org_fp.readlines()
    org_fp.close()

    includes_at = []

    for idx, line in enumerate(org_filelines):
        if line.strip().startswith("#include"):
            includes_at.append(idx)

    removed_idx = []

    for include_idx in includes_at:
        line = org_filelines[include_idx]

        if line.strip() == "#include \"%s\"" % (f[:-1] + ".h"):
            continue

        test_file = "".join(org_filelines[:include_idx] + org_filelines[include_idx+1:])
        write_fp = open(f, "w")
        write_fp.write(test_file)
        write_fp.close()

        build_error = os.system("./build.py > /dev/null 2>&1")

        if build_error == 0:
            removed_idx.append(include_idx)

    if len(removed_idx) == 0:
        write_fp = open(f, "w")
        write_fp.write(org_file)
        write_fp.close()
        continue

    print("\nRemoved lines in %s:" % f)

    new_file = ""
    for idx, line in enumerate(org_filelines):
        if not idx in removed_idx:
            new_file += line
        else:
            print(line.strip())

    write_fp = open(f, "w")
    write_fp.write(new_file)
    write_fp.close()

