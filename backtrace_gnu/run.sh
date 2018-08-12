#!/usr/bin/env bash

setUp() {
    mkdir -p /tmp/sut
    rm -f /tmp/sut/*.so
    rm -f /tmp/sut/caller
}

# $1: callee cxxflags
# $2: caller cxxflags
run() {
    # see :
    # man ld
    # -l namespec
    # --library=namespec
    # Add the archive or object file specified by namespec to the list of files to
    # link.  This option may be used any number of times.  If namespec is of the form
    # :filename, ld will search the library path for a file called filename, otherwise
    # it will search the library path for a file called libnamespec.a.
    c++ ${1} -std=c++14 -rdynamic -shared -fPIC -o /tmp/sut/callee.so ./callee.cpp
    c++ ${2} -std=c++14 -rdynamic ./caller.cpp -o /tmp/sut/caller \
    -Wl,-l:callee.so \
    -Wl,-L/tmp/sut \
    -Wl,-rpath=/tmp/sut
    /tmp/sut/caller
}

run

echo -n -e "\n\n\n////////////////////////\n\n\n"

# note this does not make a difference
# gdb's rich stacktrace relies on its own parser
run "-ggdb3" "-ggdb3" 

