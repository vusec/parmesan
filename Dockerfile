FROM ubuntu:20.04

RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections
RUN DEBIAN_FRONTEND=noninteractive apt-get update && \
    apt-get -y upgrade && \
    apt-get install -y git build-essential wget zlib1g-dev golang-go python3-pip python3-dev python-is-python3 build-essential cmake && \
    #apt-get install -y git build-essential wget zlib1g-dev golang-go python-pip python-dev build-essential cmake && \
    apt-get clean


ENV RUSTUP_HOME=/usr/local/rustup \
    CARGO_HOME=/usr/local/cargo \
    PIN_ROOT=/pin-3.7-97619-g0d0c92f4f-gcc-linux \
    GOPATH=/go \
    PATH=/clang+llvm/bin:/usr/local/cargo/bin:/parmesan/bin/:/go/bin:$PATH \
    LD_LIBRARY_PATH=/clang+llvm/lib:$LD_LIBRARY_PATH

RUN mkdir -p parmesan
COPY . parmesan
WORKDIR parmesan

RUN ./build/install_rust.sh
RUN PREFIX=/ ./build/install_llvm.sh
RUN ./build/install_tools.sh
RUN ./build/build.sh
#RUN ./build/install_pin_mode.sh
# ParmeSan does not support PIN atm

VOLUME ["/data"]
WORKDIR /data
#ENTRYPOINT [ "/opt/env.init" ]
