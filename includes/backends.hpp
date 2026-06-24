#pragma once

#include "backends/vulkan/helpers.hpp"
#include "common.hpp"
#include "surface.hpp"

namespace mjt {

#define CREATE_BACK_CREATION_ERROR_CONSTR(TYPE, ERR_TYPE)                      \
                                                                               \
  static auto create_##TYPE(ERR_TYPE &&error) -> BackendCreationError {        \
    return {ErrorType::TYPE, InnerVariant{std::move(error)}};                  \
  }

struct BackendCreationError : public IError {
  struct NoInnerErrorVariant {};
  enum class ErrorType {
    vulkan_failed_to_load_instance,  // Stored in variant 0
    vulkan_failed_to_select_device,  // Stored in variant 0
    vulkan_failed_to_build_device,   // Stored in variant 0
    surface_creation_error,          // Stored in variant 1
    volk_initialization_error,       // Stored in variant 2
    vulkan_no_queue_available,       // Stored in variant 3
  };

  using InnerVariant = std::
    variant<std::error_code, VkSurfaceError, VulkanError, NoInnerErrorVariant>;

  BackendCreationError(ErrorType type_in, InnerVariant err_in)
      : type(type_in), error(err_in) {}

  ErrorType type;
  InnerVariant error;

  CREATE_BACK_CREATION_ERROR_CONSTR(
    vulkan_failed_to_load_instance,
    std::error_code);
  CREATE_BACK_CREATION_ERROR_CONSTR(
    vulkan_failed_to_select_device,
    std::error_code);
  CREATE_BACK_CREATION_ERROR_CONSTR(
    vulkan_failed_to_build_device,
    std::error_code);
  CREATE_BACK_CREATION_ERROR_CONSTR(surface_creation_error, VkSurfaceError);
  CREATE_BACK_CREATION_ERROR_CONSTR(volk_initialization_error, VulkanError);
  CREATE_BACK_CREATION_ERROR_CONSTR(
    vulkan_no_queue_available,
    NoInnerErrorVariant);

  auto to_string() const -> std::string override final {

    switch (type) {
      case ErrorType::vulkan_failed_to_load_instance:
        return fmt::format(
          "Vulkan failed to load instance, error {} -> {}",
          std::get<0>(error).value(),
          std::get<0>(error).message());

      case ErrorType::vulkan_failed_to_select_device:
        return fmt::format(
          "Vulkan failed to select device, error {} -> {}",
          std::get<0>(error).value(),
          std::get<0>(error).message());

      case ErrorType::vulkan_failed_to_build_device:
        return fmt::format(
          "Vulkan failed to build device, error {} -> {}",
          std::get<0>(error).value(),
          std::get<0>(error).message());

      case ErrorType::surface_creation_error:
        return std::get<1>(error).to_string();
      case ErrorType::volk_initialization_error:
        return std::get<2>(error).to_string();
      case ErrorType::vulkan_no_queue_available:
        return "Vulkan, failed to find available queues.";
    }
  }
};

#undef CREATE_BACK_CREATION_ERROR_CONSTR

}  // namespace mjt