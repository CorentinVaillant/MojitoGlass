#pragma once

#include <doctest/doctest.h>

#include "surface/sdl_surface.hpp"

TEST_CASE("SDL surface creation") {
  SdlSurfaceParams params = SdlSurfaceParams::create_vulkan_presset("test");

  auto surface_result = SdlSurface::create(params);
  CHECK_MESSAGE(surface_result.is_ok(),
                "Error while creating sdl surface got : ",
                surface_result.unwrap_err().to_string());

  SUBCASE("Second SDL surface creation") {
    // Should fail
    auto second_surface = SdlSurface::create(params);
    CHECK_MESSAGE(second_surface.is_err(), "Should get an error !");
  }
}