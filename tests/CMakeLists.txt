add_executable(MojitoGlass_tests
    main.cpp
)

target_include_directories(MojitoGlass_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/includes"
)

target_link_libraries(MojitoGlass_tests PRIVATE
    MojitoGlass::MojitoGlass
    doctest::doctest          
)

target_compile_options(MojitoGlass_tests PRIVATE -Wall)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/libs/doctest/scripts/cmake")
include(doctest)              
doctest_discover_tests(MojitoGlass_tests)