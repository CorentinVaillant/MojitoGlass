#include "./backends/vulkan/queue.hpp"
#include "backends/vulkan/helpers.hpp"
#include "backends/vulkan/semaphore.hpp"
#include "backends/vulkan/swapchain.hpp"
#include "common.hpp"
#include <mutex>
#include <utility>

namespace mjt {
namespace vk {

//===== VulkanQueue =====//

//== Constructors ==//

Queue::Queue(Queue &&rval) noexcept {
  copy(rval);
  rval.nullify();
}

auto Queue::operator=(Queue &&rval) noexcept -> Queue & {
  if (this != &rval) {
    copy(rval);
    rval.nullify();
  }
  return *this;
}

auto Queue::copy(const Queue &other) noexcept -> void {
  this->device      = other.device;
  this->alloc_calls = other.alloc_calls;
  this->queue       = other.queue;
  this->family_idx  = other.family_idx;
  this->caps        = other.caps;
}

auto Queue::nullify() noexcept -> void {
  device      = VK_NULL_HANDLE;
  alloc_calls = nullptr;
  queue       = VK_NULL_HANDLE;
  family_idx  = 0;
  caps        = {};
}

//== Methods ==//

auto Queue::present(
  Swapchain &swapchain,
  uint32_t image_index,
  Semaphore *semaphore) -> VulkanResult<> {

  // VkResult *results                  = new VkResult[SWAPCHAIN_COUNT];
  VkResult result;
  VkResult *results          = &result;
  VkSemaphore wait_semaphore = semaphore ? semaphore->raw() : VK_NULL_HANDLE;
  VkPresentInfoKHR info{
    .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext              = nullptr,
    .waitSemaphoreCount = semaphore ? 1u : 0u,
    .pWaitSemaphores    = semaphore ? &wait_semaphore : nullptr,
    .swapchainCount     = 1,
    .pSwapchains        = &swapchain.raw(),
    .pImageIndices      = &image_index,
    .pResults           = results,
  };

  auto present_result = VULKAN_RESULT(vkQueuePresentKHR(queue, &info));
  // delete [] results;
  return present_result;
}

auto Queue::wait_idle() -> VulkanResult<> {
  return VULKAN_RESULT(vkQueueWaitIdle(queue));
}

auto Queue::create_cmd_pool(CmdPoolCreateFlags create_flags)
  -> VulkanResult<CmdPool> {

  VkCommandPoolCreateInfo create_info{
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext            = nullptr,
    .flags            = create_flags.flags,
    .queueFamilyIndex = family_idx,
  };

  return CmdPool::create(device, queue, alloc_calls, &create_info);
}

//===== VulkanQueuePool =====//

//== Constructors ==//
QueuePool::Handle::Handle(Handle &&rval) noexcept
    : queue(rval.queue), pool(rval.pool) {
  rval.queue = nullptr;
  rval.pool  = nullptr;
}

auto QueuePool::Handle::operator=(Handle &&rval) noexcept -> Handle & {
  if (this != &rval) {
    this->pool  = rval.pool;
    this->queue = rval.queue;

    rval.pool   = nullptr;
    rval.queue  = nullptr;
  }
  return *this;
}

QueuePool::Handle::~Handle() {
  if (queue && pool)
    pool->release(queue);
}

auto QueuePool::operator=(QueuePool &&rval) noexcept
  -> QueuePool & {
  if (this != &rval) {
    this->families = std::move(rval.families);
    this->slots    = std::move(rval.slots);
    this->mutex    = std::move(rval.mutex);
  }
  return *this;
}

QueuePool::QueuePool(std::vector<QueueFamily> &&fams)
    : families(std::move(fams)), mutex(std::make_unique<std::mutex>()) {
  for (auto &family : families) {
    for (auto &queue : family.queues)
      slots.push_back({&queue, false});
  }
}

//== Methods ==//
auto QueuePool::release(Queue *q) -> void {
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

auto QueuePool::acquire(QueueFlags required, bool presentable)
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

auto QueuePool::try_acquire(QueueFlags required, bool presentable)
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

auto QueuePool::dedicated_family(QueueFlagBit cap) const
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

auto QueuePool::all_family_indices() const -> std::vector<uint32_t> {
  std::vector<uint32_t> result;
  result.reserve(families.size());
  for (auto &f : families) {
    result.push_back(f.family_index);
  }
  return result;
}
}
}  // namespace mjt