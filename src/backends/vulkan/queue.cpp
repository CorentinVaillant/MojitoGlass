#include "./backends/vulkan/queue.hpp"
#include "backends/vulkan/helpers.hpp"
#include "backends/vulkan/semaphore.hpp"
#include "backends/vulkan/swapchain.hpp"
#include "common.hpp"
#include <mutex>
#include <utility>

namespace mjt {

//===== VulkanQueue =====//

//== Constructors ==//

VulkanQueue::VulkanQueue(VulkanQueue &&rval) noexcept {
  copy(rval);
  rval.nullify();
}

auto VulkanQueue::operator=(VulkanQueue &&rval) noexcept -> VulkanQueue & {
  if (this != &rval) {
    copy(rval);
    rval.nullify();
  }
  return *this;
}

auto VulkanQueue::copy(const VulkanQueue &other) noexcept -> void {
  this->queue      = other.queue;
  this->family_idx = other.family_idx;
  this->caps       = other.caps;
}

auto VulkanQueue::nullify() noexcept -> void {
  queue      = VK_NULL_HANDLE;
  family_idx = 0;
  caps       = {};
}

//== Methods ==//
auto VulkanQueue::submit(
  VkCommandBuffer cmd,
  VulkanFence *signal_fence,
  VulkanSemaphore *wait_semaphore,
  VulkanSemaphore *signal_semaphore,
  VulkanPipelineStages wait_stages,
  bool protected_submit) -> VulkanResult<> {

  VkSemaphoreSubmitInfo wait_submit_info;
  if (wait_semaphore)
    wait_submit_info = wait_semaphore->submit_info(0, wait_stages.flags);

  VkCommandBufferSubmitInfo cmd_submit_info{
    .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
    .pNext         = nullptr,
    .commandBuffer = cmd,
    .deviceMask    = 0,
  };

  VkSemaphoreSubmitInfo signal_submit_info;
  if (signal_semaphore)
    signal_submit_info = signal_semaphore->submit_info(0, wait_stages.flags);

  VkSubmitInfo2 info{
    .sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
    .pNext                  = nullptr,
    .flags                  = protected_submit ? VK_SUBMIT_PROTECTED_BIT : 0x0u,
    .waitSemaphoreInfoCount = wait_semaphore ? 1u : 0u,
    .pWaitSemaphoreInfos    = wait_semaphore ? &wait_submit_info : nullptr,
    .commandBufferInfoCount = 1,
    .pCommandBufferInfos    = &cmd_submit_info,
    .signalSemaphoreInfoCount = signal_semaphore ? 1u : 0u,
    .pSignalSemaphoreInfos = signal_semaphore ? &signal_submit_info : nullptr,
  };

  return VULKAN_RESULT(vkQueueSubmit2(
    queue, 1, &info, signal_fence ? signal_fence->raw() : VK_NULL_HANDLE));
}

auto VulkanQueue::present(
  VulkanSwapchain &swapchain,
  uint32_t image_index,
  VulkanSemaphore *semaphore) -> VulkanResult<> {

  // VkResult *results                  = new VkResult[SWAPCHAIN_COUNT];
  VkResult result;
  VkResult *results = &result;
  VkPresentInfoKHR info{
    .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext              = nullptr,
    .waitSemaphoreCount = semaphore ? 1u : 0u,
    .pWaitSemaphores    = semaphore ? &semaphore->raw() : nullptr,
    .swapchainCount     = 1,
    .pSwapchains        = &swapchain.raw(),
    .pImageIndices      = &image_index,
    .pResults           = results,
  };

  auto present_result = VULKAN_RESULT(vkQueuePresentKHR(queue, &info));
  // delete [] results;
  return present_result;
}

auto VulkanQueue::wait_idle() -> VulkanResult<> {
  return VULKAN_RESULT(vkQueueWaitIdle(queue));
}

//===== VulkanQueuePool =====//

//== Constructors ==//
VulkanQueuePool::Handle::Handle(Handle &&rval) noexcept
    : queue(rval.queue), pool(rval.pool) {
  rval.queue = nullptr;
  rval.pool  = nullptr;
}

auto VulkanQueuePool::Handle::operator=(Handle &&rval) noexcept -> Handle & {
  if (this != &rval) {
    this->pool  = rval.pool;
    this->queue = rval.queue;

    rval.pool   = nullptr;
    rval.queue  = nullptr;
  }
  return *this;
}

VulkanQueuePool::Handle::~Handle() {
  if (queue && pool)
    pool->release(queue);
}

auto VulkanQueuePool::operator=(VulkanQueuePool &&rval) noexcept
  -> VulkanQueuePool & {
  if (this != &rval) {
    this->families = std::move(rval.families);
    this->slots    = std::move(rval.slots);
    this->mutex    = std::move(rval.mutex);
  }
  return *this;
}

VulkanQueuePool::VulkanQueuePool(std::vector<QueueFamily> &&fams)
    : families(std::move(fams)), mutex(std::make_unique<std::mutex>()) {
  for (auto &family : families) {
    for (auto &queue : family.queues)
      slots.push_back({&queue, false});
  }
}

//== Methods ==//
auto VulkanQueuePool::release(VulkanQueue *q) -> void {
  {
    std::unique_lock lock(*mutex);
    for (auto &slot : slots) {
      if (slot.queue == q) {
        slot.in_use = false;
        break;
      }
    }
  }
  cv->notify_one();
}

auto VulkanQueuePool::acquire(VulkanQueueFlags required, bool presentable)
  -> std::optional<Handle> {
  bool exist = false;
  for (auto &slot : slots) {
    if (
      (slot.queue->capabilities().flags.flags & required.flags)
        == required.flags
      && (/*p1 => p2*/ !presentable || slot.queue->capabilities().present)) {
      exist = true;
      break;
    }
  }

  if (!exist)
    return std::nullopt;

  std::unique_lock lock(*mutex);

  cv->wait(lock, [&] {
    for (auto &slot : slots)
      if (
        !slot.in_use
        && (
      (slot.queue->capabilities().flags.flags & required.flags)
        == required.flags
      && (/*p1 => p2*/ !presentable || slot.queue->capabilities().present)))
        return true;
    return false;
  });
  for (auto &slot : slots) {
    if (
      !slot.in_use
      && (
      (slot.queue->capabilities().flags.flags & required.flags)
        == required.flags
      && (/*p1 => p2*/ !presentable || slot.queue->capabilities().present))) {
      slot.in_use = true;
      return Handle(slot.queue, this);
    }
  }
  LOGERR("Unreachable");
  return std::nullopt;
}

auto VulkanQueuePool::try_acquire(VulkanQueueFlags required, bool presentable)
  -> std::optional<Handle> {
  std::unique_lock lock(*mutex);
  for (auto &slot : slots) {
    if (
      !slot.in_use
      && (
      (slot.queue->capabilities().flags.flags & required.flags)
        == required.flags
      && (/*p1 => p2*/ !presentable || slot.queue->capabilities().present))) {
      slot.in_use = true;
      return Handle(slot.queue, this);
    }
  }
  return std::nullopt;
}

auto VulkanQueuePool::dedicated_family(VulkanQueueFlagBit cap) const
  -> std::optional<uint32_t> {
  const uint32_t cap_bit = static_cast<uint32_t>(cap);

  for (auto &family : families)
    if (family.capabilities.flags.flags == cap_bit)
      return family.family_index;

  for (auto &family : families)
    if (family.capabilities.flags.flags & cap_bit)
      return family.family_index;
  return std::nullopt;
}

auto VulkanQueuePool::all_family_indices() const -> std::vector<uint32_t> {
  std::vector<uint32_t> result;
  result.reserve(families.size());
  for (auto &f : families) {
    result.push_back(f.family_index);
  }
  return result;
}

}  // namespace mjt