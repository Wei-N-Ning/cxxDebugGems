
add_library(backtrace_local_static STATIC
    src/atomic.c
    src/backtrace.c
    include/backtrace.h
    src/dwarf.c
    src/elf.c
    src/fileline.c
    src/mmap.c
    src/mmapio.c
    src/posix.c
    src/print.c
    src/simple.c
    src/sort.c
    src/state.c
    )
target_include_directories(backtrace_local_static
    PUBLIC
    include
    PRIVATE
    src
    )
set_target_properties(backtrace_local_static
    PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    )
