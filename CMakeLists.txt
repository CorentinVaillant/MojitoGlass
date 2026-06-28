cmake_minimum_required(VERSION 3.12)

project(MojitoGlass VERSION 0.1.0 LANGUAGES CXX)

find_package(Vulkan REQUIRED)

option(MOJITO_BUILD_SHARED "Build MojitoGlass as a shared library" OFF)
option(BUILD_TESTING "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

message(STATUS "Compiler      : ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ standard  : ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type    : ${CMAKE_BUILD_TYPE}")
message(STATUS "Shared lib    : ${MOJITO_BUILD_SHARED}")
message(STATUS "Tests         : ${BUILD_TESTING}")
message(STATUS "Examples      : ${BUILD_EXAMPLES}")

add_subdirectory(libs)
add_subdirectory(src)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
