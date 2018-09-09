
add_tinytest_executable(private_go_public
    private_go_public.cpp)
set_target_properties(private_go_public
    PROPERTIES
    CXX_STANDARD 14)
add_test(NAME "subvert-class-access::private-go-public"
    COMMAND private_go_public)