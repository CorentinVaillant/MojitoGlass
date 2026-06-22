#pragma once

#include "./default_values.hpp"
#include "math/vec2.hpp"

namespace mjt {

constexpr bool DEFAULT_VALIDATION_LAYER =
#ifndef NDEBUG
  true;
#else
  false;
#endif

// ===== DeviceExtensions ===== //
struct VulkanDeviceExtensions {

  VkPhysicalDeviceAccelerationStructureFeaturesKHR acc_struct_features{
    .sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
    .pNext                                                 = nullptr,
    .accelerationStructure                                 = VK_FALSE,
    .accelerationStructureCaptureReplay                    = VK_FALSE,
    .accelerationStructureIndirectBuild                    = VK_FALSE,
    .accelerationStructureHostCommands                     = VK_FALSE,
    .descriptorBindingAccelerationStructureUpdateAfterBind = VK_FALSE,
  };

  VkPhysicalDeviceRayTracingPipelineFeaturesKHR rt_pipeline_features{
    .sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
    .pNext                                                 = nullptr,
    .rayTracingPipeline                                    = VK_FALSE,
    .rayTracingPipelineShaderGroupHandleCaptureReplay      = VK_FALSE,
    .rayTracingPipelineShaderGroupHandleCaptureReplayMixed = VK_FALSE,
    .rayTracingPipelineTraceRaysIndirect                   = VK_FALSE,
    .rayTraversalPrimitiveCulling                          = VK_FALSE,
  };
  // TODO add all the required features

  VulkanDeviceExtensions() { chain(); }

  VulkanDeviceExtensions(const VulkanDeviceExtensions &other) {
    memcpy((void *)this, &other, sizeof(other));
    chain();
  }

  VulkanDeviceExtensions &operator=(const VulkanDeviceExtensions &other) {
    if (this != &other) {
      memcpy((void *)this, &other, sizeof(other));
      chain();
    }
    return *this;
  }

private:
  void chain() {
    acc_struct_features.pNext = &rt_pipeline_features;
    // TODO continu the chain with the other members added
  }

public:
  // Iterator over device extensions

  struct VoidPtrDeviceExtIterator {
    void *ptr               = nullptr;

    using iterator_category = std::forward_iterator_tag;
    using value_type        = void *;
    using difference_type   = std::ptrdiff_t;
    using pointer           = void **;
    using reference         = void *&;

    inline void *operator*() const { return ptr; }

    VoidPtrDeviceExtIterator &operator++() {
      if (ptr)
        ptr = reinterpret_cast<VkBaseOutStructure *>(ptr)->pNext;
      return *this;
    }

    inline bool operator!=(const VoidPtrDeviceExtIterator &other) const {
      return ptr != other.ptr;
    }

    inline bool operator==(const VoidPtrDeviceExtIterator &other) const {
      return ptr == other.ptr;
    }
  };

  VoidPtrDeviceExtIterator begin() {
    return VoidPtrDeviceExtIterator(&acc_struct_features);
  }

  VoidPtrDeviceExtIterator end() { return VoidPtrDeviceExtIterator(nullptr); }
};

// ===== VkBackendBuilder ===== //
struct VulkanBackendBuilder {
  std::string app_name      = "Vk app";
  bool use_validation_layer = DEFAULT_VALIDATION_LAYER;
  UVec2 minimum_version     = {1, 3};
  VkPhysicalDeviceVulkan11Features physical_device_Vk11_feature =
    DEFAULT_PHYSICAL_DEVICE_11_FEATURES;
  VkPhysicalDeviceVulkan12Features physical_device_Vk12_feature =
    DEFAULT_PHYSICAL_DEVICE_12_FEATURES;
  VkPhysicalDeviceVulkan13Features physical_device_Vk13_feature =
    DEFAULT_PHYSICAL_DEVICE_13_FEATURES;
  VkPhysicalDeviceVulkan14Features physical_device_Vk14_feature =
    DEFAULT_PHYSICAL_DEVICE_14_FEATURES;

  std::vector<const char *> extensions = DEFAULT_EXTENSIONS;

  VulkanDeviceExtensions device_extensions;
};
}  // namespace mjt