#pragma once

#include "backends/vulkan/command_pool.hpp"
#include "backends/vulkan/semaphore.hpp"
#include "backends/vulkan/swapchain.hpp"
#include <optional>
#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"
#include "helpers.hpp"

namespace mjt {
namespace vk {

class QueuePool;
class Backend;

//===== QueueCapabilities =====//

///@brief reduce set of supported `VkQueueFlagBits` enumerations
enum class QueueFlagBit : uint32_t {
  Graphics = VK_QUEUE_GRAPHICS_BIT,
  Compute  = VK_QUEUE_COMPUTE_BIT,
  Transfer = VK_QUEUE_TRANSFER_BIT,
};

using QueueFlags = EnumFlagsWrapper<VkQueueFlags, QueueFlagBit>;

struct QueueCapabilities {
  QueueFlags flags;
  bool present;
};

//===== VulkanQueue =====//

class Queue {
  friend QueuePool;
  friend Backend;
  //== Attributs ==//
  VkDevice device                          = VK_NULL_HANDLE;
  const VkAllocationCallbacks *alloc_calls = nullptr;
  VkQueue queue                            = VK_NULL_HANDLE;

  uint32_t family_idx                      = 0;
  QueueCapabilities caps                   = {};

  //== Constructors ==//
  Queue(
    VkDevice device_,
    const VkAllocationCallbacks *allocator_callback,
    VkQueue q,
    uint32_t family,
    QueueCapabilities c)
      : device(device_), alloc_calls(allocator_callback), queue(q),
        family_idx(family), caps(c) {}

public:
  NO_COPY(Queue);
  Queue(Queue &&) noexcept;
  auto operator=(Queue &&) noexcept -> Queue &;
  ~Queue() noexcept = default;  // This is not owned, the familly is
private:
  auto copy(const Queue &other) noexcept -> void;
  auto nullify() noexcept -> void;
  //== Methods ==//
public:
  auto present(
    Swapchain &swapchain,
    uint32_t image_index,
    Semaphore * = nullptr) -> VulkanResult<>;
  auto wait_idle() -> VulkanResult<>;

  auto create_cmd_pool(CmdPoolCreateFlags create_flags)
    -> VulkanResult<CmdPool>;

  // Getters
  inline uint32_t family_index() const { return family_idx; }
  inline QueueCapabilities capabilities() const { return caps; }
  inline VkQueue raw() const { return queue; }
};

//===== QueueFamily =====//
struct QueueFamily {
  uint32_t family_index;
  QueueCapabilities capabilities;
  std::vector<Queue> queues;
};
//===== QueuePool =====//

class QueuePool {
  friend Backend;
  //== Inner types ==//
private:
  struct Slot {
    Queue *queue = nullptr;
    bool in_use        = false;
  };

public:
  class Handle {
    friend QueuePool;
    //= Attributs =//
  private:
    Queue *queue    = nullptr;
    QueuePool *pool = nullptr;
    //= Constructors =//
  public:
    NO_COPY(Handle);
    Handle(Handle &&) noexcept;
    auto operator=(Handle &&) noexcept -> Handle &;
    ~Handle();

  private:
    Handle(Queue *q, QueuePool *p) : queue(q), pool(p) {}

    //= Getters =//
  public:
    inline auto get() -> Queue & { return *queue; }
    inline auto get() const -> const Queue & { return *queue; }
    inline auto operator->() -> Queue * { return queue; }
    inline auto operator->() const -> const Queue * { return queue; }
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
  NO_COPY(QueuePool);
  QueuePool(QueuePool &&) noexcept = default;
  auto operator=(QueuePool &&) noexcept -> QueuePool &;

  ~QueuePool() = default;

private:
  QueuePool(std::vector<QueueFamily> &&fams);
  //== Methods ==//
private:
  auto release(Queue *q) -> void;

public:
  ///@brief acquire a queue with `required` capabilities.
  /// Will waits if all of the corresponding queues are already taken.
  ///@return `nullopt` if no such queue exist.
  auto acquire(QueueFlags required, bool presentable = false)
    -> std::optional<Handle>;

  ///@brief acquire a queue with `required` capabilities.
  ///@return `nullopt` if no such queue exist or if all the corresponding queue
  /// are taken.
  auto try_acquire(QueueFlags required, bool presentable = false)
    -> std::optional<Handle>;

  ///@brief return the dedicated family to a capability
  ///@return `nullopt` if the familly does not exist.
  auto dedicated_family(QueueFlagBit cap) const
    -> std::optional<uint32_t>;

  ///@return all the familly indices present.
  auto all_family_indices() const -> std::vector<uint32_t>;
};
}
};  // namespace mjt