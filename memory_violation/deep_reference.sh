#!/usr/bin/env bash

# this is to show how to use address-sanitizer to
# quickly identify explicit/implicit memory violation

CC=${CC-gcc}
CXX=${CXX-g++}
DBG=${DBG-gdb}

# happy path

${CXX} -std=c++14 -fsanitize=address -g ./deep_reference.cpp \
-o /tmp/normal
/tmp/normal

# sad path 1
# double free does not terminate the program but it is a bad practice
# asan reports very clearly what is causing it

${CXX} -DDOUBLEFREE -std=c++14 -fsanitize=address -g ./deep_reference.cpp \
-o /tmp/doublefree
/tmp/doublefree

# sad path 2
# asan reports the stack trace after segfault (GDB can do better if
# the program carries debug symbol)

${CXX} -DSEGFAULT -std=c++14 -fsanitize=address -g ./deep_reference.cpp \
-o /tmp/segfault
/tmp/segfault
