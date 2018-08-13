
add_library(libcallee_libbt SHARED
    callee.cpp
    impl.cpp)
set_target_properties(libcallee_libbt
    PROPERTIES
    CXX_STANDARD 14)
target_link_libraries(libcallee_libbt
    dl
    backtrace
)

add_executable(bt
    caller.cpp)
set_target_properties(bt
    PROPERTIES
    CXX_STANDARD 14)
target_link_libraries(bt
    PUBLIC
    libcallee_libbt)

