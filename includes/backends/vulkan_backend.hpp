#pragma once

#include "backends/vulkan/command_buffer.hpp"
#include "backends/vulkan/fence.hpp"
#include "backends/vulkan/helpers.hpp"
#include "backends/vulkan/queue.hpp"
#include "backends/vulkan/semaphore.hpp"
#include "backends/vulkan/swapchain_builder.hpp"
#include "backends/vulkan/vulkan_backend_infos.hpp"
#include "vulkan/formats.hpp"
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
namespace vk {

// ===== VulkanBackend ===== //

class Backend {
  //== Attributs ==//
  bool nulled = true;  // Set to true if the backend is not valid
  static constexpr VkAllocationCallbacks *allocator =
    nullptr;  // > set to null for now, mayber change in the future to add
              // custom allocator
  VkInstance instance                      = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
  uint32_t api_version;

  VkSurfaceKHR surface                                  = VK_NULL_HANDLE;
  VkSurfaceCapabilities2KHR surface_caps                = {};
  std::vector<VkSurfaceFormat2KHR> surface_formats      = {};

  VkPhysicalDevice physical_device                      = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties physical_device_properties = {};

  VkDevice device                                       = VK_NULL_HANDLE;

  VmaVulkanFunctions vma_functions                      = {};

  std::unique_ptr<QueuePool> pool                 = nullptr;

  //== Constructors ==//
  Backend() = default;
  NO_COPY(Backend);

  void nullify() noexcept;
  auto copy(const Backend &other) noexcept -> void;
  auto move(Backend &other) noexcept -> void;

public:
  Backend(Backend &&rval) noexcept;
  auto operator=(Backend &&rval) noexcept -> Backend &;

  static auto create(BackendBuilder &builder, IVkSurface &surface)
    -> Result<Backend, BackendCreationError>;

  ~Backend() noexcept;

private:
  auto init(BackendBuilder &builder, IVkSurface &surface)
    -> std::optional<BackendCreationError>;

  // tuple : (index, count, properties)
  auto init_queue_pool(
    std::span<std::tuple<uint32_t, uint32_t, VkQueueFamilyProperties>>
      queue_families_props) -> VulkanResult<>;

  // == Methods == //
public:
  ///@brief
  auto get_info() const {
    auto result = BackendInfos{
      .apiVersion    = api_version,
      .driverVersion = physical_device_properties.driverVersion,
      .vendorID      = physical_device_properties.vendorID,
      .deviceID      = physical_device_properties.deviceID,
      .physical_device_type =
        static_cast<PhysicalDeviceType>(physical_device_properties.deviceType),
      .device_name = physical_device_properties.deviceName,
      .pipeline_cache_uuid =
        std::to_array(physical_device_properties.pipelineCacheUUID),
      .limits            = physical_device_properties.limits,
      .sparse_properties = physical_device_properties.sparseProperties,
      .min_image_count   = surface_caps.surfaceCapabilities.minImageCount,
      .max_image_count   = surface_caps.surfaceCapabilities.maxImageCount,
      .current_extent =
        extent_to_vec(surface_caps.surfaceCapabilities.currentExtent),
      .min_image_extent =
        extent_to_vec(surface_caps.surfaceCapabilities.minImageExtent),
      .max_image_extent =
        extent_to_vec(surface_caps.surfaceCapabilities.maxImageExtent),
      .max_image_array_layers =
        surface_caps.surfaceCapabilities.maxImageArrayLayers,
      .supported_transforms =
        {surface_caps.surfaceCapabilities.supportedTransforms},
      .current_transform = surface_caps.surfaceCapabilities.currentTransform,
      .supported_composite_alpha =
        {surface_caps.surfaceCapabilities.supportedCompositeAlpha},
      .supported_usage_flags =
        surface_caps.surfaceCapabilities.supportedUsageFlags,
      .supported_format = {}};

    for (auto &format : surface_formats)
      result.supported_format.emplace_back(
        std::pair<Format, ColorSpace>{
          Format(format.surfaceFormat.format),
          ColorSpace(format.surfaceFormat.colorSpace)});

    return result;
  }

  // -- Queue pool
  auto queue_pool() -> QueuePool & { return *pool; }
  auto queue_pool() const -> const QueuePool & { return *pool; }

  // -- Memory allocator
  auto create_memory_allocator(AllocatorCreateFlags flags) const
    -> VulkanResult<MemoryAllocator>;

  // -- Fence
  auto create_fence(bool signaled = false) const -> VulkanResult<Fence>;

  // -- Cmd
  auto create_imediate_cmd(CmdPool &pool) -> VulkanResult<ImediateCmd> {
    auto fence_ret = create_fence();
    if (!fence_ret)
      return VulkanResult<ImediateCmd>::err(fence_ret.unwrap_err());
    auto cmd_ret = pool.create_cmd(true);
    if (!cmd_ret)
      return VulkanResult<ImediateCmd>::err(cmd_ret.unwrap_err());

    return VulkanResult<ImediateCmd>::ok(
      ImediateCmd(std::move(cmd_ret.unwrap()), std::move(fence_ret.unwrap())));
  }

  // -- Semaphore
  auto create_binary_semaphore() -> VulkanResult<BinarySemaphore> {
    return BinarySemaphore::create(device, allocator);
  }
  auto create_timeline_semaphore(uint64_t initial_value)
    -> VulkanResult<TimelineSemaphore> {
    return TimelineSemaphore::create(device, allocator, initial_value);
  }

  // -- Swapchain
  auto create_swapchain_builder() -> SwapchainBuilder {
    return SwapchainBuilder(
      device, surface, allocator, surface_caps, surface_formats);
  }
};
}  // namespace vk
}  // namespace mjt