#pragma once

#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"
#include "helpers.hpp"

namespace mjt {

class VulkanFence {
  //== Attributs ==//
  VkDevice device                        = VK_NULL_HANDLE;
  const VkAllocationCallbacks *ptr_alloc = VK_NULL_HANDLE;
  VkFence fence                          = VK_NULL_HANDLE;

  //== Contructors ==//
  NO_COPY(VulkanFence);

  VulkanFence(
    VkDevice device_,
    const VkAllocationCallbacks *ptr_alloc_,
    VkFence fence_)
      : device(device_), ptr_alloc(ptr_alloc_), fence(fence_) {}

public:
  static auto create(
    VkDevice device,
    const VkFenceCreateInfo *create_info,
    const VkAllocationCallbacks *allocator) -> VulkanResult<VulkanFence> {
    VkFence fence;
    return VULKAN_RESULT(vkCreateFence(device, create_info, allocator, &fence))
      .replace_ok(VulkanFence(device, allocator, fence));
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
  ///@brief wait for the fence to be unsignal by calling vkWaitForFences
  ///@param timeout timeout in nanoseconds
  ///@return
  /// - Ok(true) if succesfully wait
  /// - Ok(false) if timeout
  /// - Err(...) if an other error than `VK_TIMEOUT` is returned
  auto wait(uint64_t timeout) const -> VulkanResult<bool> {
    auto result =
      VULKAN_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, timeout));
    if (result.is_ok())
      return VulkanResult<bool>::ok(true);
    else if (result.unwrap_err().get_returned_code() == VK_TIMEOUT)
      return VulkanResult<bool>::ok(false);
    else
      return VulkanResult<bool>::err(result.unwrap_err());
  }

  ///@brief return if the fence is signaled, by using vkGetFenceStatus
  ///@return
  /// - Ok(true) signaled
  /// - Ok(false) if not
  /// - Err(...) if an other error than `VK_NOT_READY` is returned
  auto signaled() -> VulkanResult<bool> const {
    auto result = VULKAN_RESULT(vkGetFenceStatus(device, fence));
    if (result.is_ok())
      return VulkanResult<bool>::ok(true);
    else if (result.unwrap_err().get_returned_code() == VK_NOT_READY)
      return VulkanResult<bool>::ok(false);
    else
      return VulkanResult<bool>::err(result.unwrap_err());
  }

  ///@brief reset the fence signaled state
  auto reset() -> VulkanResult<> {
    return VULKAN_RESULT(vkResetFences(device, 1, &fence));
  }
};
}  // namespace mjt