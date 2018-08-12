
# export BFD_INCLUDEDIR and BFD_LIBRARYDIR
include(${PROJECT_SOURCE_DIR}/vendor/bfd_.cmake)

add_library(bfd_static_gcc54 INTERFACE)
set_target_properties(bfd_static_gcc54
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES
    ${BFD_INCLUDEDIR}
    )
target_link_libraries(bfd_static_gcc54
    INTERFACE
    ${BFD_LIBRARYDIR}
    )

add_executable(backtraceBFD main.cpp)
set_target_properties(backtraceBFD
    PROPERTIES
    CXX_STANDARD 14)
target_link_libraries(backtraceBFD
    PUBLIC
    ld
    bfd_static_gcc54
    )

