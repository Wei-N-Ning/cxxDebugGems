
# program and solutions:
# https://stackoverflow.com/questions/5096881/does-set-target-properties-in-cmake-override-cmake-cxx-flags
# I found using COMPILE_FLAGS and link to pthread causes a linking error

# flags  example:
# https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual


add_executable(obvious_interleaving
    obvious_interleaving.cpp)
set(CMAKE_CXX_FLAGS
    "-std=c++14 -fsanitize=thread -fPIC")

add_executable(obvious_interleaving_fixed
    obvious_interleaving_fixed.cpp)
