#pragma once

#define VK_NO_PROTOTYPES
#include <volk/volk.h>

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
  auto copy(const VulkanBackend &other) noexcept -> void;

public:
  static auto create(VulkanBackendBuilder &builder, IVkSurface &surface)
    -> Result<VulkanBackend, BackendCreationError>;

  // -- Memory allocator
  auto create_memory_allocator(AllocatorCreateFlags flags) const
    -> VulkanMemoryAllocator;
};

}  // namespace mjt