#include "./backend_test.hpp"

// #include "backends/vulkan_backend.hpp"
// #include "common.hpp"
// #include "surface/sdl_surface.hpp"


// auto vk_backend_test() -> void {
//   LOG(INFO, "VkBackend test :");

//   {
//     SdlSurfaceParams params =
//         SdlSurfaceParams::create_vulkan_presset("test_app");

//     auto surface = SdlSurface::create(params).unwrap();

//     VkBackendBuilder builder;
//     builder.app_name = "test_app";
//     builder.use_validation_layer = true;
//     auto backend_result = VkBackend::create(builder, surface);
//     ASSERT_ERR(backend_result,
//                "Error while creating vulkan backend got : \n\t{}",
//                backend_result.unwrap_err());

//     LOGOK("VkBackend creation");
//   }
//   LOGOK("VkBackend destruction");
// }