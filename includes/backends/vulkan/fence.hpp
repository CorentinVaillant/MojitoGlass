#pragma once

#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"
#include "helpers.hpp"

namespace mjt {

// -- Forward declarations
class VulkanBackend;

class VulkanFence {
  friend VulkanBackend;
  //== Attributs ==//
  VkDevice device                        = VK_NULL_HANDLE;
  const VkAllocationCallbacks *ptr_alloc = VK_NULL_HANDLE;
  VkFence fence                          = VK_NULL_HANDLE;

  //== Contructors ==//
  NO_COPY(VulkanFence);
  VulkanFence(
    VkDevice device_,
    const VkFenceCreateInfo *create_info,
    const VkAllocationCallbacks *allocator_)
      : device(device_), ptr_alloc(allocator_) {

    VK_CHECK(vkCreateFence(device, create_info, ptr_alloc, &fence));
  }

  VulkanFence(VulkanFence &&rval) noexcept {
    copy(rval);
    rval.nullify();
  }

  auto operator=(VulkanFence &&rval) noexcept -> VulkanFence & {
    if (this != &rval) {
      copy(rval);
      rval.nullify();
    }
    return *this;
  }

public:
  ~VulkanFence() noexcept {
    if (device) {
      vkDestroyFence(device, fence, ptr_alloc);
      nullify();
    }
  }

private:
  auto nullify() noexcept -> void {
    device = VK_NULL_HANDLE;
    fence  = VK_NULL_HANDLE;
  }

  //== Methods ==//
  auto copy(const VulkanFence &other) noexcept -> void {
    this->fence     = other.fence;
    this->device    = other.device;
    this->ptr_alloc = other.ptr_alloc;
  }

public:
  struct WaitSuccess {};
  struct Timeout final : public IError {
    auto to_string() const -> std::string final override {
      return "Fence timeout";
    }
  };

  ///@brief wait for the fence to be unsignal by calling vkWaitForFences
  ///@param timeout timeout in nanoseconds
  auto wait(uint64_t timeout) const {
    auto result = vkWaitForFences(device, 1, &fence, VK_TRUE, timeout);
    if (result == VK_TIMEOUT)
      return Result<WaitSuccess, Timeout>::err({});
    else
      VK_CHECK(result);
    return Result<WaitSuccess, Timeout>::ok({});
  }

  auto signaled() const {
    auto result = vkGetFenceStatus(device, fence);
    if (result == VK_SUCCESS)
      return true;
    else if (result == VK_NOT_READY)
      return false;
    else
      VK_CHECK(result);

    // Should not be returned
    return false;
  }

  auto reset() { VK_CHECK(vkResetFences(device, 1, &fence)); }
};
}  // namespace mjt