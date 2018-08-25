cmake_minimum_required(VERSION 3.8)
project(C++DebuggingTechniques)

enable_testing()

# ///////////////////////////
# backtrace techniques:
#
# libunwind
# is an utility that can work on multiple platforms;
# it implements (or wraps) gnu libc's backtrace/backtrace_symbols()
# function
#
# libbfd (or simply BFD):
# binary file descriptor library is an utility that interprets various
# binary file formats, noticeably ELF and DWARF

# addr2line:
# a program in gnu binutils project (binutils-gdb) that takes an ELF
# executable and an address (or multiple addresses) then searches for
# the corresponding source filename and line number;
# it uses BFD

# ////////// NOTE ///////////
# the backtrace examples below requires debug info - the easiest way
# is to switch cmake's build type to debug:
# cmake -DCMAKE_BUILD_TYPE=Debug ..

find_package(Boost 1.65)
if (Boost_FOUND)
    add_subdirectory(backtrace_boost)
endif ()

add_subdirectory(backtrace_gnu)

# source:
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
#add_subdirectory(backtrace_bfd)

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
#add_subdirectory(addr2line_gnu)



