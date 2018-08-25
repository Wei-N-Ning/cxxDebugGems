cmake_minimum_required(VERSION 3.8)
project(C++DebuggingTechniques)

enable_testing()

# ////////// NOTE ///////////
# the backtrace examples below requires debug info - the easiest way
# is to switch cmake's build type to debug:
# cmake -DCMAKE_BUILD_TYPE=Debug ..

find_package(Boost 1.65)
if (Boost_FOUND)
    add_subdirectory(backtrace_boost)
endif (Boost_FOUND)

add_subdirectory(backtrace_gnu)
add_subdirectory(backtrace_libbt)

# lib bfd (binary file descriptor) is part of GNU binutils
# it seems to be a side product in GDB's building process -
# I have not figured out its position in the dependency graph in
# this project
# once built this (static) library and its sole header can be copied
# outside as a standalone library
# see this post for how to use it:
# https://oroboro.com/printing-stack-traces-file-line/
#add_subdirectory(backtrace_bfd)

# this is only my attempt to extract addr2line from GNU's binutils
# project and build it as a standalone program/library
# however it heavily depends on the bits and pieces from binutils;
# refactoring can be very difficult, at least not without lots of
# code localizations
# leave it as a future plan
#add_subdirectory(addr2line_gnu)



