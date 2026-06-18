#include "backends/vulkan_backend.hpp"

#include "VkBootstrap.h"
#include "backends.hpp"
#include "common.hpp"

auto VkBackend::create(VkBackendBuilder &builder, IVkSurface &surface)
    -> Result<VkBackend, BackendCreationError> {

  VkBackend result{};

  auto error = result.init(builder, surface);

  if (error)
    return Result<VkBackend, BackendCreationError>::err(error.value());
  else
    return Result<VkBackend, BackendCreationError>::ok(std::move(result));
}

auto VkBackend::init(VkBackendBuilder &builder, IVkSurface &surface_interface)
    -> std::optional<BackendCreationError> {
  nulled = false;
  LOG(INFO, "Init VkBackend");

  // Building instance
  vkb::InstanceBuilder inst_builder;
  auto inst_ret = inst_builder.set_app_name(builder.app_name.c_str())
                      .request_validation_layers(builder.use_validation_layer)
                      .use_default_debug_messenger()
                      //...
                      .build();

  if (!inst_ret)
    return {BackendCreationError::create_vulkan_failed_to_load_instance(
        inst_ret.error())};
  auto vkb_instance = inst_ret.value();
  instance = vkb_instance.instance;
  debug_messenger = vkb_instance.debug_messenger;

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
          .set_minimum_version(builder.minimum_version.x,
                               builder.minimum_version.y)
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

  LOG(DEBUG, "VkBackend init.");
  return {};
}

VkBackend::VkBackend(VkBackend &&rval) {
  this->instance = rval.instance;
  rval.nullify();
}
auto VkBackend::operator=(VkBackend &&rval) -> VkBackend & {
  if (this == &rval) {
    instance = rval.instance;

    rval.nullify();
  }
  return *this;
}

auto VkBackend::nullify() noexcept -> void {
  nulled = true;
  instance = VK_NULL_HANDLE;
}
VkBackend::~VkBackend() noexcept {
  if (nulled)
    return;

  // ...

  if (physical_device)
    physical_device = VK_NULL_HANDLE; // No destructors for physical device
  if (surface)
    vkDestroySurfaceKHR(instance, surface, allocator);
  if (instance)
    vkDestroyInstance(instance, allocator);
  nullify();
  LOG(INFO, "VkBackend destroyed");
}
