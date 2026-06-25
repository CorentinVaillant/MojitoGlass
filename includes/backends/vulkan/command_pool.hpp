#pragma once

#include "backends/vulkan/command_buffer.hpp"
#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"
#include "helpers.hpp"

namespace mjt {

enum class CmdPoolCreateFlagBit : uint32_t {
  ///@brief specifies that command buffers allocated from the pool will be
  /// short-lived, meaning that they will be reset or freed in a relatively
  /// short
  /// timeframe. This flag may be used by the implementation to control memory
  /// allocation behavior within the pool
  TransientBit = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
  ///@brief allows any command buffer allocated from a pool to be individually
  /// reset to the initial state; either by calling vkResetCommandBuffer, or via
  /// the implicit reset when calling vkBeginCommandBuffer. If this flag is not
  /// set on a pool, then vkResetCommandBuffer must not be called for any
  /// command
  /// buffer allocated from that pool.
  ResetCommandBufferBit = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
  ///@brief specifies that command buffers allocated from the pool are protected
  /// command buffers.
  ProtectedBit = VK_COMMAND_POOL_CREATE_PROTECTED_BIT,
};

using CmdPoolCreateFlags =
  EnumFlagsWrapper<VkCommandPoolCreateFlags, CmdPoolCreateFlagBit>;

///@brief A wrapper for a Vulkan Command Pool
class VulkanCmdPool {

  VkDevice device                            = VK_NULL_HANDLE;
  VkQueue queue                              = VK_NULL_HANDLE;
  const VkAllocationCallbacks *ptr_allocator = nullptr;
  VkCommandPool cmd_pool                     = VK_NULL_HANDLE;

  //== Contstructors ==//
  NO_COPY(VulkanCmdPool);
  VulkanCmdPool(
    VkDevice device_,
    VkQueue queue_,
    VkCommandPool cmd_pool_,
    const VkAllocationCallbacks *ptr_allocator_)
      : device(device_), queue(queue_), ptr_allocator(ptr_allocator_),
        cmd_pool(cmd_pool_) {}

public:
  static auto create(
    VkDevice device,
    VkQueue queue,
    const VkAllocationCallbacks *ptr_allocator,
    const VkCommandPoolCreateInfo *ptr_create_info)
    -> VulkanResult<VulkanCmdPool> {
    VkCommandPool cmd_pool;
    return VULKAN_RESULT(vkCreateCommandPool(
                           device, ptr_create_info, ptr_allocator, &cmd_pool))
      .replace_ok(VulkanCmdPool(device, queue, cmd_pool, ptr_allocator));
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
    if (device != VK_NULL_HANDLE)
      vkDestroyCommandPool(device, cmd_pool, ptr_allocator);
    nullify();
  }

private:
  auto copy(const VulkanCmdPool &other) noexcept -> void {
    this->device        = other.device;
    this->queue         = other.queue;
    this->cmd_pool      = other.cmd_pool;
    this->ptr_allocator = other.ptr_allocator;
  }

  auto nullify() noexcept -> void {
    device        = VK_NULL_HANDLE;
    queue         = VK_NULL_HANDLE;
    cmd_pool      = VK_NULL_HANDLE;
    ptr_allocator = nullptr;
  }

  //== Methods ==//
public:
  ///@brief Resetting a command pool recycles all of the resources from all of
  /// the command buffers allocated from the command pool back to the command
  /// pool. All command buffers that have been allocated from the command pool
  /// are put in the initial state.
  auto reset(bool reset_release_resources = false) -> VulkanResult<> {
    return VULKAN_RESULT(vkResetCommandPool(
      device,
      cmd_pool,
      reset_release_resources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT
                              : 0x0));
  }

  auto create_cmd(bool primary = false) -> VulkanResult<VulkanCmd> {
    VkCommandBufferAllocateInfo alloc_info{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext              = nullptr,
      .commandPool        = cmd_pool,
      .level              = primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY
                                    : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
      .commandBufferCount = 1,
    };

    return VulkanCmd::create(device, queue, cmd_pool, &alloc_info);
  }

};

}  // namespace mjt