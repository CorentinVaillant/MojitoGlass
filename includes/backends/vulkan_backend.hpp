#pragma once

#include "backends/vulkan/command_buffer.hpp"
#include "backends/vulkan/fence.hpp"
#include "backends/vulkan/helpers.hpp"
#include "backends/vulkan/queue.hpp"
#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "./backends/vulkan/helpers.hpp"
#include "./vulkan/vma_usages.hpp"
#include "backends.hpp"
#include "backends/vulkan/allocator.hpp"
#include "common.hpp"
#include "surface.hpp"
#include "vulkan/backend_builder.hpp"

namespace mjt {

// ===== VulkanBackend ===== //

class VulkanBackend {
  // == Attributs == //
  bool nulled = true;  // Set to true if the backend is not valid
  static constexpr VkAllocationCallbacks *allocator =
    nullptr;  // > set to null for now, mayber change in the future to add
              // custom allocator
  VkInstance instance                      = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
  uint32_t api_version;

  VkSurfaceKHR surface                                  = VK_NULL_HANDLE;

  VkPhysicalDevice physical_device                      = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties physical_device_properties = {};

  VkDevice device                                       = VK_NULL_HANDLE;

  VmaVulkanFunctions vma_functions                      = {};

  std::unique_ptr<VulkanQueuePool> pool                 = nullptr;

  // == Constructors == //
  VulkanBackend() = default;
  NO_COPY(VulkanBackend);

  void nullify() noexcept;

public:
  VulkanBackend(VulkanBackend &&rval) noexcept;
  auto operator=(VulkanBackend &&rval) noexcept -> VulkanBackend &;

  ~VulkanBackend() noexcept;

  // == Methods == //
private:
  auto init(VulkanBackendBuilder &builder, IVkSurface &surface)
    -> std::optional<BackendCreationError>;

  // tuple : (index, count, properties)
  auto init_queue_pool(
    std::span<std::tuple<uint32_t, uint32_t, VkQueueFamilyProperties>>
      queue_families_props) -> VulkanResult<>;
  auto copy(const VulkanBackend &other) noexcept -> void;
  auto move(VulkanBackend &other) noexcept -> void;

public:
  static auto create(VulkanBackendBuilder &builder, IVkSurface &surface)
    -> Result<VulkanBackend, BackendCreationError>;

  // -- Queue pool
  auto queue_pool() -> VulkanQueuePool & { return *pool; }
  auto queue_pool() const -> const VulkanQueuePool & { return *pool; }

  // -- Memory allocator
  auto create_memory_allocator(AllocatorCreateFlags flags) const
    -> VulkanResult<VulkanMemoryAllocator>;

  // -- Fence
  auto create_fence(bool signaled = false) const -> VulkanResult<VulkanFence>;

  auto create_imediate_cmd(VulkanCmdPool &pool) -> VulkanResult<ImediateCmd> {
    auto fence_ret = create_fence();
    if (!fence_ret)
      return VulkanResult<ImediateCmd>::err(fence_ret.unwrap_err());
    auto cmd_ret = pool.create_cmd(true);
    if (!cmd_ret)
      return VulkanResult<ImediateCmd>::err(cmd_ret.unwrap_err());

    return VulkanResult<ImediateCmd>::ok(
      ImediateCmd(std::move(cmd_ret.unwrap()), std::move(fence_ret.unwrap())));
  }
};

}  // namespace mjt