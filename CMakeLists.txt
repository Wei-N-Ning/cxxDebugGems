cmake_minimum_required(VERSION 3.8)
project(C++DebuggingTechniques)

enable_testing()

# ////////// NOTE ///////////
# the backtrace examples below requires debug info - the easiest way
# is to switch cmake's build type to debug:
# cmake -DCMAKE_BUILD_TYPE=Debug ..
#add_subdirectory(backtrace_boost)
add_subdirectory(backtrace_gnu)
add_subdirectory(backtrace_libbt)
#add_subdirectory(backtrace_bfd)
#add_subdirectory(addr2line_gnu)



