#pragma once

#include "backends/vulkan/fence.hpp"
#include "backends/vulkan/semaphore.hpp"
#include "backends/vulkan/swapchain.hpp"
#include <optional>
#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"
#include "helpers.hpp"

namespace mjt {

class VulkanQueuePool;
class VulkanBackend;

//===== QueueCapabilities =====//

///@brief reduce set of supported `VkQueueFlagBits` enumerations
enum class VulkanQueueFlagBit : uint32_t {
  Graphics = VK_QUEUE_GRAPHICS_BIT,
  Compute  = VK_QUEUE_COMPUTE_BIT,
  Transfer = VK_QUEUE_TRANSFER_BIT,
};

using VulkanQueueFlags = EnumFlagsWrapper<VkQueueFlags, VulkanQueueFlagBit>;

struct QueueCapabilities {
  VulkanQueueFlags flags;
  bool present;
};

//===== VulkanQueue =====//

class VulkanQueue {
  friend VulkanQueuePool;
  friend VulkanBackend;
  //== Attributs ==//
  VkQueue queue          = VK_NULL_HANDLE;
  uint32_t family_idx    = 0;
  QueueCapabilities caps = {};

  //== Constructors ==//
  VulkanQueue(VkQueue q, uint32_t family, QueueCapabilities c)
      : queue(q), family_idx(family), caps(c) {}

public:
  NO_COPY(VulkanQueue);
  VulkanQueue(VulkanQueue &&) noexcept;
  auto operator=(VulkanQueue &&) noexcept -> VulkanQueue &;
  ~VulkanQueue() noexcept = default;  // This is not owned, the familly is
private:
  auto copy(const VulkanQueue &other) noexcept -> void;
  auto nullify() noexcept -> void;
  //== Methods ==//
public:
  auto submit(
    VkCommandBuffer cmd,
    VulkanFence *signal_fence         = nullptr,
    VulkanSemaphore *wait_semaphore   = nullptr,
    VulkanSemaphore *signal_semaphore = nullptr,
    VulkanPipelineStages wait_stages  = VulkanPipelineStage::TopOfPipe,
    bool protected_submit             = false) -> VulkanResult<>;

  auto present(
    VulkanSwapchain &swapchain,
    uint32_t image_index,
    VulkanSemaphore * = nullptr) -> VulkanResult<>;
  auto wait_idle() -> VulkanResult<>;

  // Getters
  inline uint32_t family_index() const { return family_idx; }
  inline QueueCapabilities capabilities() const { return caps; }
  inline VkQueue raw() const { return queue; }
};

//===== QueueFamily =====//
struct QueueFamily {
  uint32_t family_index;
  QueueCapabilities capabilities;
  std::vector<VulkanQueue> queues;
};
//===== VulkanQueuePool =====//

class VulkanQueuePool {
  friend VulkanBackend;
  //== Inner types ==//
private:
  struct Slot {
    VulkanQueue *queue = nullptr;
    bool in_use        = false;
  };

public:
  class Handle {
    friend VulkanQueuePool;
    //= Attributs =//
  private:
    VulkanQueue *queue    = nullptr;
    VulkanQueuePool *pool = nullptr;
    //= Constructors =//
  public:
    NO_COPY(Handle);
    Handle(Handle &&) noexcept;
    auto operator=(Handle &&) noexcept -> Handle &;
    ~Handle();

  private:
    Handle(VulkanQueue *q, VulkanQueuePool *p) : queue(q), pool(p) {}

    //= Getters =//
  public:
    inline auto get() -> VulkanQueue & { return *queue; }
    inline auto get() const -> const VulkanQueue & { return *queue; }
    inline auto operator->() -> VulkanQueue * { return queue; }
    inline auto operator->() const -> const VulkanQueue * { return queue; }
  };
  //==  Attributs ==//
private:
  std::vector<QueueFamily> families;
  std::vector<Slot> slots;
  std::unique_ptr<std::mutex> mutex = std::make_unique<std::mutex>();
  std::unique_ptr<std::condition_variable> cv =
    std::make_unique<std::condition_variable>();

  //== Constructors ==//
public:
  NO_COPY(VulkanQueuePool);
  VulkanQueuePool(VulkanQueuePool &&) noexcept = default;
  auto operator=(VulkanQueuePool &&) noexcept -> VulkanQueuePool &;

  ~VulkanQueuePool() = default;

private:
  VulkanQueuePool(std::vector<QueueFamily> &&fams);
  //== Methods ==//
private:
  auto release(VulkanQueue *q) -> void;

public:
  ///@brief acquire a queue with `required` capabilities.
  /// Will waits if all of the corresponding queues are already taken.
  ///@return `nullopt` if no such queue exist.
  auto acquire(VulkanQueueFlags required, bool presentable = false)
    -> std::optional<Handle>;

  ///@brief acquire a queue with `required` capabilities.
  ///@return `nullopt` if no such queue exist or if all the corresponding queue
  /// are taken.
  auto try_acquire(VulkanQueueFlags required, bool presentable = false)
    -> std::optional<Handle>;

  ///@brief return the dedicated family to a capability
  ///@return `nullopt` if the familly does not exist.
  auto dedicated_family(VulkanQueueFlagBit cap) const -> std::optional<uint32_t>;

  ///@return all the familly indices present.
  auto all_family_indices() const -> std::vector<uint32_t>;
};

};  // namespace mjt