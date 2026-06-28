#pragma once

#include "backends/vulkan/formats.hpp"
#include "backends/vulkan/images_usages.hpp"
#include "backends/vulkan/swapchain_builder.hpp"
#include "math/vec2.hpp"
#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"

namespace mjt {

enum class PhysicalDeviceType {
  Other         = VK_PHYSICAL_DEVICE_TYPE_OTHER,
  IntegratedGpu = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
  DiscreteGpu   = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
  VirtualGpu    = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
  Cpu           = VK_PHYSICAL_DEVICE_TYPE_CPU,
};

//@brief Todo
struct VulkanBackendInfos {
  // Physical device properties
  uint32_t apiVersion;
  uint32_t driverVersion;
  uint32_t vendorID;
  uint32_t deviceID;
  PhysicalDeviceType physical_device_type;
  std::string device_name;
  std::array<uint8_t, VK_UUID_SIZE> pipeline_cache_uuid;
  VkPhysicalDeviceLimits limits;
  VkPhysicalDeviceSparseProperties sparse_properties;

  // Surface properties
  uint32_t min_image_count;
  uint32_t max_image_count;
  U32Vec2 current_extent;
  U32Vec2 min_image_extent;
  U32Vec2 max_image_extent;
  uint32_t max_image_array_layers;
  SurfaceTransform supported_transforms;
  SurfaceTransform current_transform;
  CompositeAlpha supported_composite_alpha;
  VulkanImageUsage supported_usage_flags;

  std::vector<std::pair<VulkanFormat, VulkanColorSpace>> supported_format;
};

}  // namespace mjt