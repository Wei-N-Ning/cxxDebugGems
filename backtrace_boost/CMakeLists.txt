
add_library(libcallee_boost SHARED
    callee.cpp)
set_target_properties(libcallee_boost
    PROPERTIES
    CXX_STANDARD 14)

add_executable(caller_boost
    caller.cpp)
set_target_properties(caller_boost
    PROPERTIES
    CXX_STANDARD 14)
target_link_libraries(caller_boost
    PUBLIC
    libcallee_boost)
