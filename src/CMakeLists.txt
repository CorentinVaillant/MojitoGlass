if(MOJITO_BUILD_SHARED)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -DVERBOSITY=INFO")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DVERBOSITY=FATAL")

add_library(MojitoGlass ${LIB_TYPE}
    backends.cpp
    backends/vulkan_backend.cpp
    surface/sdl_surface.cpp
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

