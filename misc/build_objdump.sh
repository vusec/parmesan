#!/bin/bash
export PARMESAN_BASE=$(pwd)
source $PARMESAN_BASE/parmesan.env

# Create a workdir
mkdir workdir
cd workdir
wget http://ftpmirror.gnu.org/binutils/binutils-2.34.tar.xz
# or curl -O http://ftpmirror.gnu.org/binutils/binutils-2.34.tar.xz
tar xf binutils-2.34.tar.xz
mkdir build # Create a build dir

# texinfo required to build binutils
apt-get install -y texinfo
cd binutils-2.34
CC=gclang CXX=gclang++ ./configure --with-pic
make -j$(nprocs) # Build in parallel
cd binutils/
get-bc objdump
# Will create the file objdump.bc
mkdir -p ../../build
cp objdump.bc ../../build
cd ../../build

mkdir in/
# Get some input seeds for objdump
cp /usr/bin/whoami in/
# Add small dummy file
echo "AAAAAAAA" > in/a.txt
# Build everything
python3 $PARMESAN_BASE/tools/compile_bc.py objdump.bc -s -d @@
