#pragma once

#include "backends/vulkan/formats.hpp"
#include <vulkan/vulkan_core.h>

#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"
#include "helpers.hpp"

namespace mjt {
namespace vk {

class SwapchainBuilder;

// TODO
class Swapchain {
  friend SwapchainBuilder;
  //== Attributs ==//
private:
  struct SwapchainImage {
    VkImage image;
    VkImageView view;
    VkSemaphore render_finished;
  };

  VkDevice device                              = VK_NULL_HANDLE;
  VkSwapchainKHR swapchain                     = VK_NULL_HANDLE;
  const VkAllocationCallbacks *alloc_callbacks = nullptr;
  VkSwapchainCreateInfoKHR create_infos;

  /*TODO*/ std::vector<SwapchainImage> images = {};
  /*TODO*/ VkSemaphore image_available        = VK_NULL_HANDLE;
  /*TODO*/ VkFence render_fence               = VK_NULL_HANDLE;

  //== Constructors ==//
private:
  NO_COPY(Swapchain);
  Swapchain(
    VkDevice device_,
    VkSwapchainKHR swapchain_,
    const VkAllocationCallbacks *alloc_callbacks_,
    VkSwapchainCreateInfoKHR create_infos_,
    std::vector<SwapchainImage> &&images_,
    VkSemaphore image_available_,
    VkFence render_fence_)
      : device(device_), swapchain(swapchain_),
        alloc_callbacks(alloc_callbacks_), create_infos(create_infos_),
        images(std::move(images_)), image_available(image_available_),
        render_fence(render_fence_) {}

  auto nullify() noexcept -> void {
    device          = VK_NULL_HANDLE;
    swapchain       = VK_NULL_HANDLE;
    alloc_callbacks = nullptr;
    images          = {};
    image_available = VK_NULL_HANDLE;
    render_fence    = VK_NULL_HANDLE;
  }
  auto copy(const Swapchain &other) noexcept -> void {
    this->device          = other.device;
    this->swapchain       = other.swapchain;
    this->alloc_callbacks = other.alloc_callbacks;
    this->create_infos    = other.create_infos;
    this->image_available = other.image_available;
    this->render_fence    = other.render_fence;
  }
  auto move(Swapchain &&other) noexcept -> void {
    this->images = std::move(other.images);
  }

public:
  Swapchain(Swapchain &&rval) noexcept {
    copy(rval);
    move(std::move(rval));
    rval.nullify();
  }

  auto operator=(Swapchain &&rval) noexcept -> Swapchain & {
    if (this != &rval) {
      copy(rval);
      move(std::move(rval));
      rval.nullify();
    }
    return *this;
  }

  ~Swapchain() noexcept {
    if (device != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(device, swapchain, alloc_callbacks);
      for (auto image : images) {
        //> VkImages are destroyed by vkDestroySwapchainKHR
        // vkDestroyImage(device, image.image, alloc_callbacks);
        vkDestroyImageView(device, image.view, alloc_callbacks);
        vkDestroySemaphore(device, image.render_finished, alloc_callbacks);
      }
      vkDestroySemaphore(device, image_available, alloc_callbacks);
      vkDestroyFence(device, render_fence, alloc_callbacks);
      nullify();
    }
  }

  //== Methods ==//
public:
  inline auto raw() -> VkSwapchainKHR & { return swapchain; }
};
}  // namespace vk
}  // namespace mjt