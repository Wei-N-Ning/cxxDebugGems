cmake_minimum_required(VERSION 3.8)
project(C++DebuggingTechniques)

enable_testing()

# //////////////////////////
# // backtrace techniques //
# //////////////////////////

# //////////////
# ///// 1 //////
# //////////////
# the plain old libc backtrace function:
# this can only return the frame pointers and binary file paths
# it is almost completely useless in a production environment
add_subdirectory(backtrace_gnu)

# libunwind
# is an utility that can work on multiple platforms;
# it implements (or wraps) gnu libc's backtrace/backtrace_symbols()
# function
#

# //////////////
# ///// 2 //////
# //////////////
# libbfd (or simply BFD):
# binary file descriptor library is an utility that interprets various
# binary file formats, noticeably ELF and DWARF
# I started to create an example using it but didn't finish it,
# see the notes below

# addr2line:
# a program in gnu binutils project (binutils-gdb) that takes an ELF
# executable and an address (or multiple addresses) then searches for
# the corresponding source filename and line number;
# it uses BFD
# I started to refactor addr2line out of GNU binutils but didn't finish
# it, see the notes below

# ////////// NOTE ///////////
# the backtrace examples below requires debug info - the easiest way
# is to switch cmake's build type to debug:
# cmake -DCMAKE_BUILD_TYPE=Debug ..

# //////////////
# ///// 3 //////
# //////////////
# boost 1.65+ offers its own stacktrace library:
# http://boostorg.github.io/stacktrace/index.html
# this is exactly what I need
# notes on building boost libraries
# (the debian package on gnuship - Ubuntu 16 - was 1.58.0;
# it did not contain the newly introduced stacktrace module)
# download  1.68.0 to tmp
# run ./bootstrap.sh
# run sudo ./b2 link=static to produce the static libraries;
# run sudo ./b2 link=shared to produce the shared libraries;
#
# http://boostorg.github.io/stacktrace/stacktrace/getting_started.html#stacktrace.getting_started.handle_terminates_aborts_and_seg
# http://boostorg.github.io/stacktrace/stacktrace/configuration_and_build.html
#
# interestingly boost offers two address-translation backends:
# libbacktrace (https://github.com/gcc-mirror/gcc/tree/master/libbacktrace)
# addr2line (I already knew this...)
# comparison shows that addr2line is able to identify lambda functions,
# so it looks like the winner here
find_package(Boost 1.65)
if (Boost_FOUND)
    add_subdirectory(backtrace_boost)
endif ()

# //////////////
# ///// 4 //////
# //////////////
# GCC's backtrace component (now a standalone library understand BSD
# license) - libbacktrace
# I needed to use Ian's libbacktrace to implement the address-to-line
# translation. Basically, I copied boost 1.68.0 stacktrace component's
# second implementation method (the first being addr2line) and stripped
# out all the unnecessary bits (such as support for windows)
#
# however libbacktrace was not available in wt's toolchain so I took a
# look at the source:
# https://github.com/gcc-mirror/gcc/tree/master/libbacktrace
# this project is localized in backtrace_libbt

# cmake find_file():
# https://cmake.org/cmake/help/v3.0/command/find_file.html
find_file(ELF_LINK_HEADER "link.h")
if (ELF_LINK_HEADER)
    message("found elf link.h header: ${ELF_LINK_HEADER}")
    add_subdirectory(backtrace_libbt)
endif ()

# lib bfd (binary file descriptor) is part of GNU binutils
# it seems to be a side product in GDB's building process -
# I have not figured out its position in the dependency graph in
# this project
# once built this (static) library and its sole header can be copied
# outside as a standalone library
# see this post for how to use it:
# https://oroboro.com/printing-stack-traces-file-line/

if (0)
    add_subdirectory(backtrace_bfd)
endif ()

# this is only my attempt to extract addr2line from GNU's binutils
# project and build it as a standalone program/library
# however it heavily depends on the bits and pieces from binutils;
# refactoring can be very difficult, at least not without lots of
# code localizations
# /////////////////////////
# leave it as a future plan
# /////////////////////////
# see:
# https://sourceware.org/binutils/docs/binutils/addr2line.html

if (0)
    add_subdirectory(addr2line_gnu)
endif()

# see readme file in this directory
add_subdirectory(data_races)
add_subdirectory(memory_violation)