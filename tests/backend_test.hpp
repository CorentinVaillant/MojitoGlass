#pragma once

#include <doctest/doctest.h>

#include "backends/vulkan_backend.hpp"
#include "surface/sdl_surface.hpp"

TEST_CASE("VkBackend creation + destruction ") {
  SdlSurfaceParams params = SdlSurfaceParams::create_vulkan_presset("test_app");

  auto surface = SdlSurface::create(params).unwrap();

  VkBackendBuilder builder;
  builder.app_name = "test_app";
  builder.use_validation_layer = true;
  auto backend_result = VkBackend::create(builder, surface);
  CHECK_MESSAGE(backend_result.is_ok(),
                "Error while creating vulkan backend got : \n\t",
                backend_result.unwrap_err().to_string());

  SUBCASE("Double VkBackend creation") {
    auto backend_result2 = VkBackend::create(builder, surface);
    CHECK_MESSAGE(backend_result2.is_ok(),
                  "Error while creating vulkan backend got : \n\t",
                  backend_result2.unwrap_err().to_string());
    VkBackend backend2 = std::move(backend_result2.unwrap());
  }
}
