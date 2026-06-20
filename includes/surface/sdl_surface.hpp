#pragma once

#include "common.hpp"
#include "fmt/base.h"
#include "fmt/format.h"
#include "math/vec2.hpp"
#include "surface.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <string>

enum class SingleSdlSurfaceInitFlag : uint32_t {
  AUDIO    = SDL_INIT_AUDIO,
  VIDEO    = SDL_INIT_VIDEO,
  JOYSTICK = SDL_INIT_JOYSTICK,
  HAPTIC   = SDL_INIT_HAPTIC,
  GAMEPAD  = SDL_INIT_GAMEPAD,
  EVENTS   = SDL_INIT_EVENTS,
  SENSOR   = SDL_INIT_SENSOR,
  CAMERA   = SDL_INIT_CAMERA
};

struct SdlSurfaceInitFlags {
  uint32_t flags = 0x0;

  constexpr SdlSurfaceInitFlags(
    const std::initializer_list<SingleSdlSurfaceInitFlag> &flags_in) {
    for (auto flag : flags_in) {
      flags |= static_cast<uint32_t>(flag);
    }
  }

  constexpr SdlSurfaceInitFlags(SingleSdlSurfaceInitFlag flag)
      : flags((static_cast<uint32_t>(flag))) {}
};

enum class SingleSdlWindowFlag : uint64_t {
  FULLSCREEN          = SDL_WINDOW_FULLSCREEN,
  OPENGL              = SDL_WINDOW_OPENGL,
  OCCLUDED            = SDL_WINDOW_OCCLUDED,
  HIDDEN              = SDL_WINDOW_HIDDEN,
  BORDERLESS          = SDL_WINDOW_BORDERLESS,
  RESIZABLE           = SDL_WINDOW_RESIZABLE,
  MINIMIZED           = SDL_WINDOW_MINIMIZED,
  MAXIMIZED           = SDL_WINDOW_MAXIMIZED,
  MOUSE_GRABBED       = SDL_WINDOW_MOUSE_GRABBED,
  INPUT_FOCUS         = SDL_WINDOW_INPUT_FOCUS,
  MOUSE_FOCUS         = SDL_WINDOW_MOUSE_FOCUS,
  EXTERNAL            = SDL_WINDOW_EXTERNAL,
  MODAL               = SDL_WINDOW_MODAL,
  HIGH_PIXEL_DENSITY  = SDL_WINDOW_HIGH_PIXEL_DENSITY,
  MOUSE_CAPTURE       = SDL_WINDOW_MOUSE_CAPTURE,
  MOUSE_RELATIVE_MODE = SDL_WINDOW_MOUSE_RELATIVE_MODE,
  ALWAYS_ON_TOP       = SDL_WINDOW_ALWAYS_ON_TOP,
  UTILITY             = SDL_WINDOW_UTILITY,
  TOOLTIP             = SDL_WINDOW_TOOLTIP,
  POPUP_MENU          = SDL_WINDOW_POPUP_MENU,
  KEYBOARD_GRABBED    = SDL_WINDOW_KEYBOARD_GRABBED,
  FILL_DOCUMENT       = SDL_WINDOW_FILL_DOCUMENT,
  VULKAN              = SDL_WINDOW_VULKAN,
  METAL               = SDL_WINDOW_METAL,
  TRANSPARENT         = SDL_WINDOW_TRANSPARENT,
  NOT_FOCUSABLE       = SDL_WINDOW_NOT_FOCUSABLE,
};

struct SdlWindowFlags {
  uint64_t flags = 0x0;

  constexpr SdlWindowFlags(
    const std::initializer_list<SingleSdlWindowFlag> &flags_in) {
    for (auto flag : flags_in) {
      flags |= static_cast<uint64_t>(flag);
    }
  }

  constexpr SdlWindowFlags(SingleSdlWindowFlag flag)
      : flags((static_cast<uint64_t>(flag))) {}
};

struct SdlSurfaceCreationError : IError {

  struct ExistingInstance {};
  struct SdlError {
    const char *err;
  };

  using InnerVariant = std::variant<ExistingInstance, SdlError>;

  InnerVariant error;

  SdlSurfaceCreationError(InnerVariant &&variant) : error(std::move(variant)) {}

  auto to_string() const -> std::string override final {
    size_t error_index = error.index();

    switch (error_index) {

      case 0:
        return "There already is an active instance of SdlSurface.";
        break;

      case 1:
        return fmt::format(
          "Sdl backend returned an error ({}).", std::get<1>(error).err);
        break;

      default: return "Unknow error !";
    }
  }
};

struct SdlSurfaceParams {
  SdlWindowFlags      window_flags;
  SdlSurfaceInitFlags init_flags = {SingleSdlSurfaceInitFlag::VIDEO};
  SVec2               size       = {720, 480};
  const char         *app_name;

  static auto create_vulkan_presset(const char *app_name) -> SdlSurfaceParams {
    return SdlSurfaceParams{
      .window_flags = {SingleSdlWindowFlag::VULKAN},
      .init_flags   = {SingleSdlSurfaceInitFlag::VIDEO},
      .size         = {720, 480},
      .app_name     = app_name,
    };
  }
};

class SdlSurface : public IVkSurface {
private:
  // == Attributs == //
  bool        valid_instance = false;

  SDL_Window *window;
  // == Constructors == //
private:
  SdlSurface()                                     = default;
  SdlSurface(SdlSurface &)                         = default;
  auto operator=(SdlSurface &rval) -> SdlSurface & = default;
  auto init(SdlSurfaceParams &params) -> std::optional<SdlSurfaceCreationError>;

public:
  SdlSurface(SdlSurface &&rval);
  auto operator=(SdlSurface &&rval) -> SdlSurface &;
  // == Destructors ==
public:
  ~SdlSurface() noexcept;

private:
  auto nullify() -> void;
  // == Methods == //
public:
  auto get_vk_surface(
    VkInstance                   vulkan_instance,
    const VkAllocationCallbacks *allocator)
    -> Result<VkSurfaceKHR, VkSurfaceError> override;

  static auto create(SdlSurfaceParams &params)
    -> Result<SdlSurface, SdlSurfaceCreationError>;
};