# Example: how to build Objdump

## 1) Build ParmeSan
Use the included script `build/build.sh` to build ParmeSan and the required tools.

We really recommend you install the LLVM version supplied by the `build/install_llvm.sh` script. At the end it will show the env vars that need to be set. Tip: write these to a file that you can source later (e.g. `source angora.env`).

Also install the required tools (`gclang`) using `build/install_tools.sh`.

```bash
# You might need to change this one to point to your LLVM install path
source parmesan.env
build/build.sh
export PARMESAN_BASE=$(pwd)
```

## 2) Get sources
```bash
# Create a workdir
mkdir workdir
cd workdir
wget http://ftpmirror.gnu.org/binutils/binutils-2.34.tar.xz
# or curl -O http://ftpmirror.gnu.org/binutils/binutils-2.34.tar.xz
tar xf binutils-2.34.tar.xz
mkdir build # Create a build dir
```

## 3) Build bitcode file using gclang
```bash
cd binutils-2.34
CC=gclang CXX=gclang++ CFLAGS="-fPIC" ./configure --with-pic
make -j$(nprocs) # Build in parallel
cd binutils/
get-bc objdump
# Will create the file objdump.bc
mkdir -p ../../build
cp objdump.bc ../../build
cd ../../build
```

## 4) Run ParmeSan pipeline
We have included a script `tools/build_bc.py` that runs the many commands required to get the targets and build the different target binaries.

Invoke the `build_bc.py` script with the bitcode file as first argument, followed by the command-line arguments to the target program that should be used when profiling. 

For `objdump`, you can, for example, use the `-s -d` flags. Also add `@@` in place where the input file would normally go. So the flags for objdump become `-s -d @@`. If no arguments are given, it will default to just `@@`.

The script also expects a folder called `in/` with some inputs used for profiling the target application.

```bash
mkdir in/
# Get some input seeds for objdump
cp /usr/bin/whoami in/
# Add small dummy file
echo "AAAAAAAA" > in/a.txt
# Build everything
python3 $PARMESAN_BASE/tools/compile_bc.py objdump.bc -s -d @@
# Will take a long time, go get a coffee or a beer
# ...
# After some time it will print the command you can use
# to start the fuzzing. 
```

## 5) Start fuzzing
Now you can start fuzzing using the command printed in the previous step.

```bash
# Something like: 
/path/to/parmesan/bin/fuzzer -c ./targets.pruned.json -i in -o out -t ./objdump.track -s ./objdump.san.fast -- ./objdump.fast -s -d @@
```

This should start up the fuzzer (with the sanopt optimization), and show you something like the following:

![ParmeSan Screenshot](/misc/screenshot.png)


If you do not want to fuzz it with a sanitizer enable at all, remove the `-s objdump.san.fast` flag. Alternatively, you can also fuzz the target with the sanitizer always enabled. Simply replace `objdump.fast` with `objdump.san.fast` in that case.
