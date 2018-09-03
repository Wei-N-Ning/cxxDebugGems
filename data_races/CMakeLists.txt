
# program and solutions:
# https://stackoverflow.com/questions/5096881/does-set-target-properties-in-cmake-override-cmake-cxx-flags
# I found using COMPILE_FLAGS and link to pthread causes a linking error

# flags  example:
# https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual

add_executable(obvious_interleaving
    obvious_interleaving.cpp)
set(CMAKE_CXX_FLAGS
    "-std=c++14 -fsanitize=thread -fPIC")

# wrap the mutable data in std::atomic
# this gets rid of the sanitizer warning message
# but the design is still fundamentally wrong
add_executable(obvious_interleaving_nowarning
    obvious_interleaving_nowarning.cpp)

add_executable(obvious_interleaving_fixed
    obvious_interleaving_fixed.cpp)
