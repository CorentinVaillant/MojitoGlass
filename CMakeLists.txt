cmake_minimum_required(VERSION 3.12)

project(MojitoGlass)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory(src)
add_subdirectory(libs)


target_include_directories(MojitoGlass PUBLIC
    "./includes"
)

message(STATUS "Compiler : ${CMAKE_CXX_COMPILER}")
