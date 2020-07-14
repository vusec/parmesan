#!/bin/bash

set -euxo pipefail

#wllvm and gllvm
pip3 install --upgrade pip==9.0.3
pip3 install wllvm
mkdir ${HOME}/go
go get github.com/SRI-CSL/gllvm/cmd/...

#llvm-diff-parmesan
#(cd tools/llvm-diff-parmesan && mkdir build && cd build && cmake .. && cmake --build . && cp llvm-diff-parmesan ../../../bin/)
