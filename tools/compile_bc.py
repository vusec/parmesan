#!/usr/bin/env python
import sys
import os
from pathlib import Path

import logging
logging.basicConfig(stream=sys.stderr, level=logging.INFO)

SCRIPT_PATH = Path(os.path.dirname(__file__))
BIN_PATH = (Path(SCRIPT_PATH) / "../bin/").resolve()
FUZZER_PATH = BIN_PATH / "fuzzer"
CC_BIN = BIN_PATH / "angora-clang"
CXX_BIN = BIN_PATH / "angora-clang++"
DIFF_BIN = BIN_PATH / "llvm-diff-parmesan"
ID_ASSIGNER_PATH = BIN_PATH / "pass/libLLVMIDAssigner.so"
PRUNE_SCRIPT_PATH = SCRIPT_PATH / "prune.py"
CFLAGS = os.environ.get("CFLAGS", "-fPIC") # -fPIC required for targets like binutils
CXXFLAGS = os.environ.get("CXXFLAGS", "-fPIC") # -fPIC required for targets like binutils

def run_cmd(cmd):
    logging.info(f" + {cmd}")
    os.system(cmd)

def build_pipeline(bc_file, target_flags="@@", profiling_input_dir="in", is_cpp=False):
    compiler = CXX_BIN if is_cpp else CC_BIN
    cflags = CXXFLAGS if is_cpp else CFLAGS
    name = os.path.splitext(bc_file)[0]
    sanitizer = "address"
    targets_file = "targets.json"
    #1) BUILD FAST LL
    run_cmd(f"USE_FAST=1 {compiler} {cflags} -S -emit-llvm -o {name}.fast.ll {bc_file}")
    #2) BUILD FAST BIN
    run_cmd(f"USE_FAST=1 {compiler} {cflags} -o {name}.fast {bc_file}")
    #3) BUILD FAST SAN LL
    run_cmd(f"USE_FAST=1 {compiler} {cflags} -fsanitize={sanitizer} -S -emit-llvm -o {name}.san.ll {bc_file}")
    #4) BUILD FAST SAN BIN
    run_cmd(f"USE_FAST=1 {compiler} {cflags} -fsanitize={sanitizer} -o {name}.san.fast {bc_file}")
    #5) BUILD TRACK BIN
    run_cmd(f"USE_TRACK=1 {compiler} {cflags} -o {name}.track {bc_file}")
    #6) Gather targets.json and target.diff
    run_cmd(f"{DIFF_BIN} -json {name}.fast.ll {name}.san.ll 2> {name}.diff")
    #7) Gather cmp.map
    run_cmd(f"opt -load {ID_ASSIGNER_PATH} -idassign -idassign-emit-cfg \
            -idassign-cfg-file cfg.dat {name}.fast.ll")
    #8) Prune targets
    run_cmd(f"python {PRUNE_SCRIPT_PATH} {targets_file} {name}.diff cmp.map {profiling_input_dir} ./{name}.track {target_flags} > targets.pruned.json")

    # Print fuzzing command
    print("You can now run your target application (with SanOpt enabled) using:")
    print()
    print(f"{FUZZER_PATH} -c ./targets.pruned.json -i {profiling_input_dir} -o out -t ./{name}.track -s ./{name}.san.fast -- ./{name}.fast {target_flags}")
    print()

def print_usage():
    print(f"Usage: {sys.argv[0]} BC_FILE [TARGET_PROG_CMD_FLAGS]")
    print("Where the BC_FILE is an llvm .bc file obtained by, for example, gclang.")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print_usage()
        sys.exit(1)
    if len(sys.argv) > 2:
        # Provide some target program flags (e.g., for objdump, give it -s -d)
        flags = ' '.join(sys.argv[2:])
        build_pipeline(sys.argv[1], target_flags=flags)
    else:
        build_pipeline(sys.argv[1])
