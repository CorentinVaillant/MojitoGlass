#pragma once

#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"
#include "helpers.hpp"

namespace mjt {
// -- Forward Declaration
class VulkanBackend;

///@brief A wrapper for a Vulkan Command Pool
class VulkanCmdPool {

  VkDevice device                            = VK_NULL_HANDLE;
  VkCommandPool cmd_pool                     = VK_NULL_HANDLE;
  const VkAllocationCallbacks *ptr_allocator = nullptr;

  //== Contstructors ==//
  NO_COPY(VulkanCmdPool);
  VulkanCmdPool(
    VkDevice device_,
    VkCommandPool cmd_pool_,
    const VkAllocationCallbacks *ptr_allocator_)
      : device(device_), cmd_pool(cmd_pool_), ptr_allocator(ptr_allocator_) {}

public:
  static auto create(
    VkDevice device,
    const VkCommandPoolCreateInfo &create_info,
    const VkAllocationCallbacks *ptr_allocator) -> VulkanResult<VulkanCmdPool> {
    VkCommandPool cmd_pool;
    return VULKAN_RESULT(vkCreateCommandPool(
                           device, &create_info, ptr_allocator, &cmd_pool))
      .replace_ok(VulkanCmdPool(device, cmd_pool, ptr_allocator));
  }

  VulkanCmdPool(VulkanCmdPool &&rval) {
    copy(rval);
    rval.nullify();
  }

  auto operator=(VulkanCmdPool &&rval) noexcept -> VulkanCmdPool & {
    if (this != &rval) {
      copy(rval);
      rval.nullify();
    }
    return *this;
  }

  ~VulkanCmdPool() noexcept {
    vkDestroyCommandPool(device, cmd_pool, ptr_allocator);
    nullify();
  }

private:
  auto copy(const VulkanCmdPool &other) noexcept -> void {
    this->device        = other.device;
    this->cmd_pool      = other.cmd_pool;
    this->ptr_allocator = other.ptr_allocator;
  }

  auto nullify() noexcept -> void {
    device        = VK_NULL_HANDLE;
    cmd_pool      = VK_NULL_HANDLE;
    ptr_allocator = nullptr;
  }
};

}  // namespace mjt