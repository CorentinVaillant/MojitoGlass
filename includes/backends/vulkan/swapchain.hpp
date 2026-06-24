#pragma once

#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"
#include "helpers.hpp"

namespace mjt {

class VulkanSwapchain {
  //== Attributs ==//
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;

  //== Constructors ==//
  NO_COPY(VulkanSwapchain);
  VulkanSwapchain() { LOGERR("Not yet implemented."); }

  //== Methods ==//
public:
  inline auto raw() -> VkSwapchainKHR & { return swapchain; }
};
}  // namespace mjt