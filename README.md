# ParmeSan: Sanitizer-guided Greybox Fuzzing

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

ParmeSan is a sanitizer-guided greybox fuzzer based on
[Angora](https://github.com/AngoraFuzzer/Angora).

## Published Work

USENIX Security 2020: [ParmeSan: Sanitizer-guided Greybox Fuzzing](https://www.usenix.org/conference/usenixsecurity20/presentation/osterlund).

The paper can be found here: [ParmeSan: Sanitizer-guided Greybox Fuzzing](https://download.vusec.net/papers/parmesan_sec20.pdf)


## Building ParmeSan
See the instructions for [Angora](https://github.com/AngoraFuzzer).

Basically run the following scripts to install the dependencies and build ParmeSan:
```bash
build/install_rust.sh
PREFIX=/path/to/install/llvm build/install_llvm.sh
build/install_tools.sh
build/build.sh
```

ParmeSan also builds a tool `bin/llvm-diff-parmesan`, which can be used for target
acquisition.

## Building a target
First build your program into a bitcode file using `clang` (e.g., base64.bc). Then build your target in the same way, but with your selected sanitizer enabled. To get a single bitcode file for larger projects, the easiest solution is to use [gllvm](https://github.com/SRI-CSL/gllvm).

```bash
# Build the bitcode files for target acquisition
USE_FAST=1 $(pwd)/bin/angora-clang -emit-llvm -o base64.fast.bc -c base64.bc
USE_FAST=1 $(pwd)/bin/angora-clang -fsanitize=address -emit-llvm -o base64.fast.asan.bc -c base64.bc
# Build the actual binaries to be fuzzed
USE_FAST=1 $(pwd)/bin/angora-clang -o base64.fast -c base64.bc
USE_TRACK=1 $(pwd)/bin/angora-clang -o base64.track -c base64.bc
```

Then acquire the targets using:
```bash
bin/llvm-diff-parmesan -json base64.fast.bc base64.fast.asan.bc
```

This will output a file `targets.json`, which you provide to ParmeSan with the `-c` flag.

For example:
```bash
$(pwd)/bin/fuzzer -c ./targets.json -i in -o out -t ./base64.track -- ./base64.fast -d @@
```

## Options
ParmeSan's SanOpt option can speed up the fuzzing process by dynamically
switching over to a sanitized binary only once the fuzzer reaches one of the
targets specified in the `targets.json` file.

Enable using the `-s [SANITIZED_BIN]` option.

Build the sanitized binary in the following way:
```bash
USE_FAST=1 $(pwd)/bin/angora-clang -fsanitize=address -o base64.asan.fast -c base64.bc
```

## Targets input file
The targets input file consisit of a JSON file with the following format:
```json
{
  "targets":  [1,2,3,4],
  "edges":   [[1,2], [2,3]],
  "callsite_dominators": {"1": [3,4,5]}
}
``` 

Where the targets denote the identify of the cmp instruction to target (i.e., the id assigned by the `__angora_trace_cmp()` calls) and edges is the overlay graph of cmp ids (i.e., which cmps are connected to each other). The `edges` filed can be empty, since ParmeSan will add newly discovered edges automatically, but note that the performance will be better if you provide the static CFG.

It is also possible to run ParmeSan in pure directed mode (`-D` option),
meaning that it will only consider new seeds if the seed triggers coverage that
is on a direct path to one of the specified targets. Note that this requires a
somewhat complete static CFG to work (an incomplete CFG might contain no paths
to the targets at all, which would mean that no new coverage will be considered
at all).

![ParmeSan Screenshot](/misc/screenshot.png)

## How to get started
Have a look at [BUILD_TARGET.md](/BUILD_TARGET.md) for a step-by-step tutorial on how to get started fuzzing with ParmeSan.

## FAQ

* Q: I get a warning like `==1561377==WARNING: DataFlowSanitizer: call to uninstrumented function gettext` when running the (track) instrumented program.
* A: In many cases you can ignore this, but it will lose the taint (meaning worse performance). You need to add the function to the abilist (e.g., `llvm_mode/dfsan_rt/dfsan/done_abilist.txt`) and add a custom DFSan wrapper (in `llvm_mode/dfsan_rt/dfsan/dfsan_custom.cc`). See the [Angora documentation](https://github.com/AngoraFuzzer/Angora/blob/master/docs/example.md) for more info.
* Q: I get an compiler error when building the track binary.
* A: ParmeSan/ Angora uses DFSan for dynamic data-flow analysis. In certain cases building target applications can be a bit tricky (especially in the case of C++ targets). Make sure to disable as much inline assembly as possible and make sure that you link the correct libraries/ llvm libc++. Some programs also do weird stuff like an indirect call to a vararg function. This is not supported by DFSan at the moment, so the easy solution is to patch out these calls, or do something like [indirect call promotion](https://llvm.org/devmtg/2015-10/slides/Baev-IndirectCallPromotion.pdf).
* Q: `llvm-diff-parmesan` generates too many targets!
* A: You can do target pruning using the scripts in `tools/` (in particular `tools/prune.py`) or use [ASAP](https://github.com/dslab-epfl/asap) to generate a target bitcode file with fewer sanitizer targets.

## Docker image
You can also get the pre-built docker image of ParmeSan.

```bash
docker pull vusec/parmesan
docker run --rm -it vusec/parmesan
# In the container you can build objdump
/parmesan/misc/build_objdump.sh
```
