#include "backends/vulkan_backend.hpp"

#include "VkBootstrap.h"
#include "backends.hpp"
#include "backends/vulkan/allocator.hpp"
#include "backends/vulkan/backend_builder.hpp"
#include "backends/vulkan/fence.hpp"
#include "backends/vulkan/helpers.hpp"
#include "backends/vulkan/queue.hpp"
#include "common.hpp"
#include <vulkan/vulkan_core.h>

namespace mjt {

auto debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
  void *) -> VkBool32 {

  const char *msg_type = vkb::to_string_message_type(messageType);
  const char *msg_name = pCallbackData->pMessageIdName;
  const char *msg      = pCallbackData->pMessage;

  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    LOGERR_NO_THROW("[{}] ({}) {}", msg_type, msg_name, msg);
    return VK_FALSE;
  } else if (
    messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    LOGWARN("[{}] ({}) {}", msg_type, msg_name, msg);
    return VK_TRUE;

  } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    LOG(INFO, "[{}] ({}) {}", msg_type, msg_name, msg);
    return VK_TRUE;

  } else if (
    messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    LOG(DEBUG, "[{}] ({}) {}", msg_type, msg_name, msg);
    return VK_TRUE;

  } else {
    LOG(NOTSET, "[{}] ({}) {}", msg_type, msg_name, msg);
  }
  return VK_TRUE;
}

auto VulkanBackend::create(VulkanBackendBuilder &builder, IVkSurface &surface)
  -> Result<VulkanBackend, BackendCreationError> {

  using Ret = Result<VulkanBackend, BackendCreationError>;

  VulkanBackend result{};

  auto error = result.init(builder, surface);

  if (error)
    return Ret::err(error.value());
  else
    return Ret::ok(std::move(result));
}

static bool volk_init = false;
auto VulkanBackend::init(
  VulkanBackendBuilder &builder,
  IVkSurface &surface_interface) -> std::optional<BackendCreationError> {
  nulled = false;
  LOG(INFO, "Init VkBackend");

  if (!volk_init) {
    auto result = VULKAN_RESULT(volkInitialize());
    if (result.is_err())
      return {BackendCreationError::create_volk_initialization_error(
        std::move(result.unwrap_err()))};
    volk_init = true;
  }

  // Building instance
  vkb::InstanceBuilder inst_builder;
  auto inst_ret =
    inst_builder.set_app_name(builder.app_name.c_str())
      .request_validation_layers(builder.use_validation_layer)
      .set_debug_callback(debug_callback)
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

  std::vector<vkb::CustomQueueDescription> queue_descs;
  // tuple : (index, count, properties)
  std::vector<std::tuple<uint32_t, uint32_t, VkQueueFamilyProperties>>
    queue_families_props;

  auto queue_families = selector_ret->get_queue_families();

  for (uint32_t i = 0; i < queue_families.size(); i++) {
    uint32_t total_count = 0;

    for (auto &queue_request : builder.queue_request) {
      if (
        queue_families[i].queueFlags & queue_request.capabilities.flags.flags) {
        uint32_t count = std::min(
          queue_request.count, queue_families[i].queueCount - total_count);
        if (count == 0)
          continue;

        total_count += count;
      }
    }

    if (total_count > 0) {
      queue_descs.emplace_back(i, std::vector<float>(total_count, 1.0f));
      queue_families_props.emplace_back(i, total_count, queue_families[i]);
    }
  }

  if (queue_descs.empty())
    return {BackendCreationError::create_vulkan_no_queue_available({})};

  auto device_ret = device_builder.custom_queue_setup(queue_descs).build();
  if (!device_ret)
    return {BackendCreationError::create_vulkan_failed_to_build_device(
      device_ret.error())};
  device = device_ret->device;
  // volkLoadDevice(device);

  auto init_q_pool_res = init_queue_pool(queue_families_props);
  if (init_q_pool_res.is_err())
    return BackendCreationError::create_vulkan_failed_to_init_queue_pool(
      std::move(init_q_pool_res.unwrap_err()));

  vma_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vma_functions.vkGetDeviceProcAddr   = vkGetDeviceProcAddr;

  LOG(DEBUG, "VkBackend init.");
  return {};
}

auto VulkanBackend::init_queue_pool(
  // tuple : (index, count, properties)
  std::span<std::tuple<uint32_t, uint32_t, VkQueueFamilyProperties>>
    queue_families_props) -> VulkanResult<> {

  using Ret = VulkanResult<>;
#define RET_IF_ERR(x)                                                          \
  do {                                                                         \
    auto res_RET_IF_ERR = VULKAN_RESULT(x);                                    \
    if (res_RET_IF_ERR.is_err())                                               \
      return Ret::err(res_RET_IF_ERR.unwrap_err());                            \
  } while (0)

  std::vector<QueueFamily> families;
  families.reserve(queue_families_props.size());

  // Building the families
  for (auto [fam_idx, count, prop] : queue_families_props) {

    VkBool32 suport_present;
    RET_IF_ERR(vkGetPhysicalDeviceSurfaceSupportKHR(
      physical_device, fam_idx, surface, &suport_present));

    QueueFamily family{};
    family.family_index             = fam_idx;
    family.capabilities.flags.flags = static_cast<uint32_t>(prop.queueFlags);
    family.capabilities.present     = suport_present == VK_TRUE;
    family.queues.reserve(count);

    // Building the family's queues
    for (uint32_t i = 0; i < count; i++) {
      VkQueue vk_queue = VK_NULL_HANDLE;
      vkGetDeviceQueue(device, fam_idx, i, &vk_queue);

      if (vk_queue == VK_NULL_HANDLE) {
        LOGWARN("Failed to get queue[{}] from family[{}]", i, fam_idx);
        continue;
      };
      family.queues.emplace_back(
        VulkanQueue(device, allocator, vk_queue, fam_idx, family.capabilities));
    }

    if (!family.queues.empty())
      families.emplace_back(std::move(family));
  }

  pool =
    std::make_unique<VulkanQueuePool>(VulkanQueuePool(std::move(families)));

  return Ret::ok({});
#undef RET_IF_ERR
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

auto VulkanBackend::move(VulkanBackend &other) noexcept -> void {
  pool = std::move(other.pool);
}

VulkanBackend::VulkanBackend(VulkanBackend &&rval) noexcept {
  copy(rval);
  move(rval);
  rval.nullify();
}
auto VulkanBackend::operator=(VulkanBackend &&rval) noexcept
  -> VulkanBackend & {
  if (this != &rval) {
    copy(rval);
    move(rval);

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
  pool                       = nullptr;
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
  -> VulkanResult<VulkanMemoryAllocator> {
  VmaAllocatorCreateInfo alloc_info{};

  alloc_info.flags                = flags.flags;
  alloc_info.physicalDevice       = physical_device;
  alloc_info.device               = device;
  alloc_info.pAllocationCallbacks = allocator;
  alloc_info.instance             = instance;
  alloc_info.vulkanApiVersion     = api_version;
  alloc_info.pVulkanFunctions     = &vma_functions;

  return VulkanMemoryAllocator::create(alloc_info);
}

// -- Fence

auto VulkanBackend::create_fence(bool signaled) const
  -> VulkanResult<VulkanFence> {
  VkFenceCreateInfo create_info{
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .pNext = nullptr,
    .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0x0u,
  };

  return VulkanFence::create(device, &create_info, allocator);
}

}  // namespace mjt