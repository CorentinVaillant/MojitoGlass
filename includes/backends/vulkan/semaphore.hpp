#pragma once

#include "common.hpp"
#include "helpers.hpp"

namespace mjt {

class VulkanSemaphore {
  //== Attributs ==//

  VkSemaphore semaphore = VK_NULL_HANDLE;
  //== Constructors ==//
  VulkanSemaphore() { LOGERR("Not implente yet"); }

  //== Methods ==//
public:
  inline auto raw() -> VkSemaphore & { return semaphore; };

  static auto raw_span(std::span<VulkanSemaphore> semaphores) {
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

}  // namespace mjt