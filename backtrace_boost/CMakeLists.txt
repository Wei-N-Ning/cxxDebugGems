
set(Boost_USE_STATIC_LIBS ON)
include(${PROJECT_SOURCE_DIR}/vendor/boost_.cmake)
find_package(Boost REQUIRED)
add_library(boost_static INTERFACE)
set_target_properties(boost_static
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES
    ${Boost_INCLUDE_DIRS})
target_link_libraries(boost_static
    INTERFACE
    ${Boost_LIBRARIES})

add_library(libcallee_boost SHARED
    callee.cpp)
set_target_properties(libcallee_boost
    PROPERTIES
    CXX_STANDARD 14)
target_link_libraries(libcallee_boost
    PRIVATE
    boost_static)

add_executable(caller_boost
    caller.cpp)
set_target_properties(caller_boost
    PROPERTIES
    CXX_STANDARD 14)
# boost/stacktrace/detail/location_from_symbol.hpp: 31
# man dladdr -> advices to link with -ldl
target_link_libraries(caller_boost
    PUBLIC
    libcallee_boost
    dl)
