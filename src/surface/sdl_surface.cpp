#include "surface/sdl_surface.hpp"

#include "SDL3/SDL_error.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "SDL3/SDL_vulkan.h"
#include "common.hpp"
#include "surface.hpp"

#include <volk.h>

namespace mjt {

static bool existing_instance = false;

// == Helper function == //
//...

//============ SdlSurface ============//
// == Constructors == //
auto SdlSurface::init(SdlSurfaceParams &params)
  -> std::optional<SdlSurfaceCreationError> {

  LOG(INFO, "Creating a SDLSurface.");
  if (existing_instance)
    return {
      SdlSurfaceCreationError{SdlSurfaceCreationError::ExistingInstance{}}};

  if (!SDL_Init(params.init_flags.flags))
    return {SdlSurfaceCreationError{
      SdlSurfaceCreationError::SdlError{SDL_GetError()}}};

  window = SDL_CreateWindow(
    params.app_name, params.size.x, params.size.y, params.window_flags.flags);
  if (!window)
    return {SdlSurfaceCreationError{
      SdlSurfaceCreationError::SdlError{SDL_GetError()}}};

  valid_instance    = true;
  existing_instance = true;

  LOG(DEBUG, "SDLSurface created.");
  return {};
}

SdlSurface::SdlSurface(SdlSurface &&rval)
    : SdlSurface(static_cast<SdlSurface &>(rval)) {
  rval.nullify();
}

auto SdlSurface::operator=(SdlSurface &&rval) -> SdlSurface & {

  if (this != &rval) {
    *this = static_cast<SdlSurface &>(rval);
    rval.nullify();
  }
  return *this;
}

SdlSurface::~SdlSurface() noexcept {
  if (!valid_instance)
    return;
  LOG(INFO, "Destroying SDLSurface.");

  SDL_DestroyWindow(window);
  nullify();
  existing_instance = false;
  LOG(INFO, "SDLSurface destroyed.");
}
auto SdlSurface::nullify() -> void {
  valid_instance = false;
  window         = nullptr;
}

// == Methods == //

auto SdlSurface::get_vk_surface(
  VkInstance instance,
  const VkAllocationCallbacks *allocator)
  -> Result<VkSurfaceKHR, VkSurfaceError> {
  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(window, instance, allocator, &surface)) {
    return Result<VkSurfaceKHR, VkSurfaceError>::err(SDL_GetError());
  }

  return Result<VkSurfaceKHR, VkSurfaceError>::ok(std::move(surface));
}

auto SdlSurface::create(SdlSurfaceParams &params)
  -> Result<SdlSurface, SdlSurfaceCreationError> {

  SdlSurface result;

  auto err_ret = result.init(params);
  if (err_ret)
    return Result<SdlSurface, SdlSurfaceCreationError>::err(err_ret.value());

  return Result<SdlSurface, SdlSurfaceCreationError>::ok(std::move(result));
}
}  // namespace mjt