#pragma once

#include "common.hpp"
#include "helpers.hpp"
#include <vulkan/vulkan_core.h>

namespace mjt {
namespace vk {

// TODO
class Semaphore {
protected:
  //== Attributs ==//
  VkDevice device                                  = VK_NULL_HANDLE;
  const VkAllocationCallbacks *allocator_callbacks = nullptr;
  VkSemaphore semaphore                            = VK_NULL_HANDLE;

  //== Constructors ==//
  Semaphore(
    VkDevice device_,
    const VkAllocationCallbacks *allocator_callbacks_,
    VkSemaphore semaphore_)
      : device(device_), allocator_callbacks(allocator_callbacks_),
        semaphore(semaphore_) {}

public:
  NO_COPY(Semaphore);
  static auto create(
    VkDevice device,
    const VkSemaphoreCreateInfo *info,
    const VkAllocationCallbacks *allocator_callbacks)
    -> VulkanResult<Semaphore> {
    VkSemaphore semaphore;
    return VULKAN_RESULT(
             vkCreateSemaphore(device, info, allocator_callbacks, &semaphore))
      .replace_ok(Semaphore{device, allocator_callbacks, semaphore});
  }

  Semaphore(Semaphore &&rval) noexcept {
    copy(rval);
    rval.nullify();
  }

  auto operator=(Semaphore &&rval) noexcept -> Semaphore & {
    if (this != &rval) {
      copy(rval);
      rval.nullify();
    }
    return *this;
  }

  ~Semaphore() noexcept {
    if (device != VK_NULL_HANDLE) {
      vkDestroySemaphore(device, semaphore, allocator_callbacks);
      nullify();
    }
  }

protected:
  auto copy(const Semaphore &other) noexcept -> void {
    this->device              = other.device;
    this->allocator_callbacks = other.allocator_callbacks;
    this->semaphore           = other.semaphore;
  }

  auto nullify() noexcept -> void {
    device              = VK_NULL_HANDLE;
    allocator_callbacks = nullptr;
    semaphore           = VK_NULL_HANDLE;
  }

  //== Methods ==//

public:
  inline auto raw() -> VkSemaphore { return semaphore; };

  static auto raw_span(std::span<Semaphore> semaphores) {
    std::vector<VkSemaphore> result;
    for (auto &semaphore : semaphores)
      result.push_back(semaphore.semaphore);
    return result;
  }

  auto submit_info(
    uint64_t value,
    VkPipelineStageFlagBits2 stage_mask = VK_PIPELINE_STAGE_2_NONE) {
    return VkSemaphoreSubmitInfo{
      .sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
      .pNext       = nullptr,
      .semaphore   = semaphore,
      .value       = value,
      .stageMask   = stage_mask,
      .deviceIndex = 0,

    };
  }
};

class BinarySemaphore : public Semaphore {
  BinarySemaphore(
    VkDevice device_,
    const VkAllocationCallbacks *allocator_callbacks_,
    VkSemaphore semaphore_)
      : Semaphore(device_, allocator_callbacks_, semaphore_) {}

public:
  static auto
  create(VkDevice device, const VkAllocationCallbacks *allocator_callbacks)
    -> VulkanResult<BinarySemaphore> {
    VkSemaphoreCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
    };
    VkSemaphore sem;
    return VULKAN_RESULT(
             vkCreateSemaphore(device, &info, allocator_callbacks, &sem))
      .replace_ok(BinarySemaphore{device, allocator_callbacks, sem});
  }
};

class TimelineSemaphore : public Semaphore {
  TimelineSemaphore(
    VkDevice device_,
    const VkAllocationCallbacks *allocator_callbacks_,
    VkSemaphore semaphore_)
      : Semaphore(device_, allocator_callbacks_, semaphore_) {}

public:
  static auto create(
    VkDevice device,
    const VkAllocationCallbacks *allocator_callbacks,
    uint64_t initial_value) -> VulkanResult<TimelineSemaphore> {

    VkSemaphoreTypeCreateInfo type_info{
      .sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
      .pNext         = nullptr,
      .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
      .initialValue  = initial_value,
    };

    VkSemaphoreCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = &type_info,
      .flags = 0,
    };
    VkSemaphore sem;
    return VULKAN_RESULT(
             vkCreateSemaphore(device, &info, allocator_callbacks, &sem))
      .replace_ok(TimelineSemaphore{device, allocator_callbacks, sem});
  }

  auto signal(uint64_t value) {
    VkSemaphoreSignalInfo info{
      .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
      .pNext     = nullptr,
      .semaphore = semaphore,
      .value     = value,
    };

    return VULKAN_RESULT(vkSignalSemaphore(device, &info));
  }

  ///@brief wait for the semaphore to take a value
  ///@param[in] value : the value to wait for
  ///@param[in] timeout : timeout in nanoseconds
  ///@return
  /// - Ok(true) if succesfully wait
  /// - Ok(false) if timeout
  /// - Err(...) if an other error than `VK_TIMEOUT` is returned
  auto wait(uint64_t value, uint64_t timeout_ns = UINT64_MAX)
    -> VulkanResult<bool> {
    VkSemaphoreWaitInfo info{
      .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
      .pNext          = nullptr,
      .flags          = 0x0u,
      .semaphoreCount = 1,
      .pSemaphores    = &semaphore,
      .pValues        = &value,
    };
    auto result = vkWaitSemaphores(device, &info, timeout_ns);

    switch (result) {
      case VK_SUCCESS: return VulkanResult<bool>::ok(true);
      case VK_TIMEOUT: return VulkanResult<bool>::ok(false);
      default:
        return VulkanResult<bool>::err(
          VulkanError(result, "vkWaitSemaphores(device, &info, timeout_ns)"));
    }
  }

  auto query_value() -> VulkanResult<uint64_t> {
    uint64_t value;
    return VULKAN_RESULT(vkGetSemaphoreCounterValue(device, semaphore, &value))
      .replace_ok(std::move(value));
  }
};
}
}  // namespace mjt