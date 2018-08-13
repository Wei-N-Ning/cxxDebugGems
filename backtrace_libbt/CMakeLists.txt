
add_library(bktce STATIC
    bktce.h
    bktce.cpp)
set_target_properties(bktce
    PROPERTIES
    CXX_STANDARD 11
    POSITION_INDEPENDENT_CODE 1
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1)
target_link_libraries(bktce
    PRIVATE
    dl
    backtrace
)
target_include_directories(bktce
    PUBLIC
    .)

add_library(callee_libbt SHARED
    callee.cpp)
set_target_properties(callee_libbt
    PROPERTIES
    CXX_STANDARD 14)
target_link_libraries(callee_libbt
    PRIVATE
    bktce
    )

add_executable(bt
    caller.cpp)
set_target_properties(bt
    PROPERTIES
    CXX_STANDARD 14)
target_link_libraries(bt
    PRIVATE
    callee_libbt
)

