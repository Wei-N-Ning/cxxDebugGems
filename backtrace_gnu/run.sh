#!/usr/bin/env bash

mkdir -p /tmp/sut

# $1: callee cxxflags
# $2: caller cxxflags
run() {
    c++ ${1} -std=c++14 -rdynamic -shared -fPIC -o /tmp/sut/libcallee.so ./callee.cpp
    c++ ${2} -std=c++14 -rdynamic ./caller.cpp -o /tmp/sut/caller \
    -Wl,-lcallee \
    -Wl,-L/tmp/sut \
    -Wl,-rpath=/tmp/sut
    /tmp/sut/caller
}

run

echo -n -e "\n\n\n////////////////////////\n\n\n"

# note this does not make a difference
# gdb's rich stacktrace relies on its own parser
run "-ggdb3" "-ggdb3" 

