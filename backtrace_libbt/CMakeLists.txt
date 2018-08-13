
add_subdirectory(libbacktrace)

add_library(bktce_self STATIC
    bktce.h
    bktce.cpp
    )
set_target_properties(bktce_self
    PROPERTIES
    CXX_STANDARD 11
    POSITION_INDEPENDENT_CODE 1
    )

# source:
# stackoverflow: combining several static libraries into one using cmake
# some important notes:
# 1) specify the location of the resulting archive file
# 2) use ar
# 3) setup WORKING_DIRECTORY
# 4) specify the dependencies of this custom target
# 5) VERY IMPORTANT: specify the linking requirement - the consumer 
#    of this target may  not know which shared library(ies) to link to, 
#    and it is NOT their responsibility
set(BKTCE_LIB ${CMAKE_BINARY_DIR}/libbktce.a)
add_custom_target(bktce_build
    COMMAND ar -x $<TARGET_FILE:backtrace_local_static>
    COMMAND ar -x $<TARGET_FILE:bktce_self>
    COMMAND ar -qcs ${BKTCE_LIB} *.o
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS backtrace_local_static bktce_self
    )
add_library(bktce STATIC IMPORTED)
add_dependencies(bktce bktce_build)
set_target_properties(bktce
    PROPERTIES
    IMPORTED_LOCATION ${BKTCE_LIB}
    )
target_include_directories(bktce
    INTERFACE
    .
    )
target_link_libraries(bktce
    INTERFACE
    dl
    )

add_library(callee_libbt SHARED
    callee.cpp
    )
set_target_properties(callee_libbt
    PROPERTIES
    CXX_STANDARD 14
    )
target_link_libraries(callee_libbt
    PRIVATE
    bktce
    )

add_executable(bt
    caller.cpp
    )
set_target_properties(bt
    PROPERTIES
    CXX_STANDARD 14
    )
target_link_libraries(bt
    PRIVATE
    callee_libbt
    )

