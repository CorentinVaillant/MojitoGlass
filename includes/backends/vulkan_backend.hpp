#pragma once

#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include <vma/vk_mem_alloc.h>

#include "./vulkan/default_values.hpp"
#include "backends.hpp"
#include "common.hpp"
#include "math/vec2.hpp"
#include "surface.hpp"

constexpr bool DEFAULT_VALIDATION_LAYER =
#ifndef NDEBUG
    true;
#else
    false;
#endif

struct VkBackendBuilder {
  std::string app_name = "Vk app";
  bool use_validation_layer = DEFAULT_VALIDATION_LAYER;
  UVec2 minimum_version = {1, 3};
  VkPhysicalDeviceVulkan11Features physical_device_Vk11_feature =
      DEFAULT_PHYSICAL_DEVICE_11_FEATURES;
  VkPhysicalDeviceVulkan12Features physical_device_Vk12_feature =
      DEFAULT_PHYSICAL_DEVICE_12_FEATURES;
  VkPhysicalDeviceVulkan13Features physical_device_Vk13_feature =
      DEFAULT_PHYSICAL_DEVICE_13_FEATURES;
  VkPhysicalDeviceVulkan14Features physical_device_Vk14_feature =
      DEFAULT_PHYSICAL_DEVICE_14_FEATURES;
  std::vector<const char *> extensions = DEFAULT_EXTENSIONS;
};

class VkBackend {
  // == Attributs == //
  bool nulled = true; // Set to true if the backend is not valid
  static constexpr VkAllocationCallbacks *allocator =
      nullptr; // > set to null for now, mayber change in the future to add
               // custom allocator
  VkInstance instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;

  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device = VK_NULL_HANDLE;

  // == Constructors == //
  VkBackend() = default;
  NO_COPY(VkBackend);

  void nullify() noexcept;

public:
  VkBackend(VkBackend &&rval);
  auto operator=(VkBackend &&rval) -> VkBackend &;

  ~VkBackend() noexcept;

  // == Methods == //
private:
  auto init(VkBackendBuilder &builder, IVkSurface &surface)
      -> std::optional<BackendCreationError>;

public:
  static auto create(VkBackendBuilder &builder, IVkSurface &surface)
      -> Result<VkBackend, BackendCreationError>;
};