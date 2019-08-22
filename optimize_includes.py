all_files = os.listdir(".")
to_optimize = []

for f in all_files:
    if not os.path.isfile(f):
        continue

    if f.endswith(".c") or f.endswith(".h"):
        to_optimize.append(f)

for f in to_optimize:
    def should_continue(l):
        return stripped[0] == "#" or stripped[0] == "/" or len(l) == 0

    with open(f, "r") as fp:
        for line in fp:
            sl = line.strip()
            if should_abort(sl):
                break

            if line