#include "backends/vulkan_backend.hpp"

#include "VkBootstrap.h"
#include "backends.hpp"
#include "backends/vulkan/allocator.hpp"
#include "backends/vulkan/fence.hpp"
#include "backends/vulkan/helpers.hpp"

namespace mjt {

auto VulkanBackend::create(VulkanBackendBuilder &builder, IVkSurface &surface)
  -> Result<VulkanBackend, BackendCreationError> {

  VulkanBackend result{};

  auto error = result.init(builder, surface);

  if (error)
    return Result<VulkanBackend, BackendCreationError>::err(error.value());
  else
    return Result<VulkanBackend, BackendCreationError>::ok(std::move(result));
}

static bool volk_init = false;
auto VulkanBackend::init(
  VulkanBackendBuilder &builder,
  IVkSurface &surface_interface) -> std::optional<BackendCreationError> {
  nulled = false;
  LOG(INFO, "Init VkBackend");

  if (!volk_init) {
    VK_CHECK(volkInitialize());
    volk_init = true;
  }

  // Building instance
  vkb::InstanceBuilder inst_builder;
  auto inst_ret =
    inst_builder.set_app_name(builder.app_name.c_str())
      .request_validation_layers(builder.use_validation_layer)
      .use_default_debug_messenger()
      .require_api_version(VK_MAKE_API_VERSION(
        0, builder.minimum_version.x, builder.minimum_version.y, 0))
      //...
      .build();

  if (!inst_ret)
    return {BackendCreationError::create_vulkan_failed_to_load_instance(
      inst_ret.error())};
  auto vkb_instance = inst_ret.value();
  instance          = vkb_instance.instance;
  volkLoadInstance(instance);
  debug_messenger = vkb_instance.debug_messenger;
  api_version     = vkb_instance.api_version;
  auto versions   = compressed_version_to_uvec4(api_version);
  LOG(
    INFO,
    "Using api version : {} -> [{}, {}, {}, {}]",
    api_version,
    versions.x,
    versions.y,
    versions.z,
    versions.w);

  // Getting surface
  auto surface_result = surface_interface.get_vk_surface(instance, allocator);
  if (!surface_result)
    return {BackendCreationError::create_surface_creation_error(
      std::move(surface_result.unwrap_err()))};
  surface = surface_result.unwrap();

  // Init the physical device
  vkb::PhysicalDeviceSelector selector(vkb_instance);
  auto selector_ret =
    selector
      //...
      .set_minimum_version(builder.minimum_version.x, builder.minimum_version.y)
      .set_required_features_11(builder.physical_device_Vk11_feature)
      .set_required_features_12(builder.physical_device_Vk12_feature)
      .set_required_features_13(builder.physical_device_Vk13_feature)
      .set_required_features_14(builder.physical_device_Vk14_feature)
      .add_required_extensions(builder.extensions)
      .set_surface(surface)
      .select();

  if (!selector_ret)
    return {BackendCreationError::create_vulkan_failed_to_select_device(
      selector_ret.error())};

  physical_device = selector_ret.value().physical_device;
  LOG(INFO, "Using {}", selector_ret.value().name);

  // Getting physical_device_properties
  physical_device_properties = selector_ret.value().properties;

  // Enableling extensions
  selector_ret.value().enable_extensions_if_present(builder.extensions);

  // Init device
  vkb::DeviceBuilder device_builder{selector_ret.value()};
  for (void *device_extension : builder.device_extensions)
    device_builder.add_pNext(device_extension);

  auto device_ret = device_builder.build();
  if (!device_ret)
    return {BackendCreationError::create_vulkan_failed_to_build_device(
      device_ret.error())};
  device = device_ret->device;
  // volkLoadDevice(device);

  vma_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vma_functions.vkGetDeviceProcAddr   = vkGetDeviceProcAddr;

  LOG(DEBUG, "VkBackend init.");
  return {};
}

auto VulkanBackend::copy(const VulkanBackend &other) noexcept -> void {

  nulled                     = other.nulled;
  instance                   = other.instance;
  debug_messenger            = other.debug_messenger;
  api_version                = other.api_version;
  surface                    = other.surface;
  physical_device            = other.physical_device;
  physical_device_properties = other.physical_device_properties;
  device                     = other.device;
  vma_functions              = other.vma_functions;
}

VulkanBackend::VulkanBackend(VulkanBackend &&rval) noexcept {
  copy(rval);
  rval.nullify();
}
auto VulkanBackend::operator=(VulkanBackend &&rval) noexcept
  -> VulkanBackend & {
  if (this != &rval) {
    copy(rval);

    rval.nullify();
  }
  return *this;
}

auto VulkanBackend::nullify() noexcept -> void {
  nulled                     = true;
  instance                   = VK_NULL_HANDLE;
  debug_messenger            = VK_NULL_HANDLE;
  api_version                = 0;
  surface                    = VK_NULL_HANDLE;
  physical_device            = VK_NULL_HANDLE;
  physical_device_properties = {};
  device                     = VK_NULL_HANDLE;
}
VulkanBackend::~VulkanBackend() noexcept {
  if (nulled)
    return;

  if (device)
    vkDestroyDevice(device, allocator);
  if (physical_device)
    physical_device = VK_NULL_HANDLE;  // No destructors for physical device
  if (surface)
    vkDestroySurfaceKHR(instance, surface, allocator);
  if (debug_messenger)
    vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, allocator);
  if (instance)
    vkDestroyInstance(instance, allocator);
  nullify();
  LOG(INFO, "VkBackend destroyed");
}

// -- Memory allocator

auto VulkanBackend::create_memory_allocator(AllocatorCreateFlags flags) const
  -> VulkanMemoryAllocator {
  VmaAllocatorCreateInfo alloc_info{};

  alloc_info.flags                = flags.flags;
  alloc_info.physicalDevice       = physical_device;
  alloc_info.device               = device;
  alloc_info.pAllocationCallbacks = allocator;
  alloc_info.instance             = instance;
  alloc_info.vulkanApiVersion     = api_version;
  alloc_info.pVulkanFunctions     = &vma_functions;

  return VulkanMemoryAllocator(alloc_info);
}

// -- Fence

auto VulkanBackend::create_fence(bool signaled) const -> VulkanFence {
  VkFenceCreateInfo create_info{
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0x0u,
  };

  return VulkanFence(device, &create_info, allocator);
}
}  // namespace mjt