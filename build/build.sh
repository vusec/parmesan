#!/bin/bash
BIN_PATH=$(readlink -f "$0")
ROOT_DIR=$(dirname $(dirname $BIN_PATH))

set -euxo pipefail

if ! [ -x "$(command -v llvm-config)"  ]; then
    ${ROOT_DIR}/build/install_llvm.sh
    export PATH=${HOME}/clang+llvm/bin:$PATH
    export LD_LIBRARY_PATH=${HOME}/clang+llvm/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
    export CC=clang
    export CXX=clang++
fi

PREFIX=${PREFIX:-${ROOT_DIR}/bin/}

cargo build
cargo build --release

rm -rf ${PREFIX}
mkdir -p ${PREFIX}
mkdir -p ${PREFIX}/lib
cp target/release/fuzzer ${PREFIX}
cp target/release/*.a ${PREFIX}/lib
cp target/release/log_reader ${PREFIX}

cd llvm_mode
rm -rf build
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_BUILD_TYPE=Release ..
make # VERBOSE=1 
make install # VERBOSE=1

#llvm-diff-parmesan
(cd ${ROOT_DIR}/tools/llvm-diff-parmesan && mkdir -p build && cd build && cmake .. && cmake --build . && cp llvm-diff-parmesan ../../../bin/)
#id-assigner-standalone (HACK)
(cd ${ROOT_DIR}/tools/llvm-diff-parmesan && mkdir -p build-pass && cd build-pass && cmake -DBUILD_STANDALONE_PASS=1 ../id-assigner-pass && cmake --build . && cp src/*.so ../../../bin/pass/)
