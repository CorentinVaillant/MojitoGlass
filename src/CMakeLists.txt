if(MOJITO_BUILD_SHARED)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -DVERBOSITY=DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DVERBOSITY=FATAL")


#! not working don't know why !#
if(${CMAKE_BUILD_TYPE} EQUAL Release)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
else()
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
endif()
message("Compiler flags : " ${CMAKE_CXX_FLAGS})

add_library(MojitoGlass ${LIB_TYPE}
    surface/sdl_surface.cpp

    backends.cpp
    backends/vulkan_backend.cpp
    backends/vulkan/queue.cpp
    backends/vulkan/vma_usages.cpp
)

add_library(MojitoGlass::MojitoGlass ALIAS MojitoGlass)

target_include_directories(MojitoGlass
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/includes>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(MojitoGlass PRIVATE -Wall -Wextra)

target_link_libraries(MojitoGlass
    PUBLIC
    Vulkan::Vulkan
    fmt::fmt
    SDL3::SDL3
    PRIVATE
    GPUOpen::VulkanMemoryAllocator
    vk-bootstrap::vk-bootstrap
    volk
)

