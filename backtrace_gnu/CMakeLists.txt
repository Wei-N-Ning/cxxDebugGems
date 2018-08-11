
add_library(libcallee_gnu SHARED
    callee.cpp)
set_target_properties(libcallee_gnu
    PROPERTIES
    CXX_STANDARD 14)

add_executable(caller_gnu
    caller.cpp)
set_target_properties(caller_gnu
    PROPERTIES
    CXX_STANDARD 14)
target_link_libraries(caller_gnu
    PUBLIC
    libcallee_gnu)
