#pragma once

#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"

struct VkSurfaceError : public IError {
  const char *error;

  VkSurfaceError(const char *error_in) : error(error_in) {}
public:
  auto to_string() const -> std::string override final {
    return fmt::format("Sdl error : {}", error);
  }
};

class IVkSurface {
public:
  virtual auto get_vk_surface(VkInstance vulkan_instance,
                              const VkAllocationCallbacks *allocator)
      -> Result<VkSurfaceKHR, VkSurfaceError> = 0;

  virtual ~IVkSurface() = default;
};