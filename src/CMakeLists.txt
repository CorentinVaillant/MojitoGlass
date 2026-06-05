
add_executable(MojitoGlass
    math/math_test.cpp
    test/main.cpp
)

target_link_libraries(MojitoGlass PRIVATE
    fmt::fmt
)
