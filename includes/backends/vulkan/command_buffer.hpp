#pragma once

#include "backends/vulkan/fence.hpp"
#include "backends/vulkan/semaphore.hpp"
#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"
#include "helpers.hpp"

namespace mjt {
namespace vk {

///@brief a wrapper for VkCommandBuffer
class CommandBuffer {

  //== Attributs ==//
  VkDevice device        = VK_NULL_HANDLE;
  VkQueue queue          = VK_NULL_HANDLE;
  VkCommandPool cmd_pool = VK_NULL_HANDLE;
  VkCommandBuffer cmd    = VK_NULL_HANDLE;

  //== Constructors ==//
  CommandBuffer(
    VkDevice device_,
    VkQueue queue_,
    VkCommandPool cmd_pool_,
    VkCommandBuffer cmd_)
      : device(device_), queue(queue_), cmd_pool(cmd_pool_), cmd(cmd_) {}

public:
  NO_COPY(CommandBuffer);
  static auto create(
    VkDevice device,
    VkQueue queue,
    VkCommandPool cmd_pool,
    const VkCommandBufferAllocateInfo *ptr_allocate_info)
    -> VulkanResult<CommandBuffer> {
    VkCommandBuffer cmd;
    return VULKAN_RESULT(
             vkAllocateCommandBuffers(device, ptr_allocate_info, &cmd))
      .replace_ok(CommandBuffer(device, queue, cmd_pool, cmd));
  }

  CommandBuffer(CommandBuffer &&rval) noexcept {
    copy(rval);
    rval.nullify();
  }

  auto operator=(CommandBuffer &&rval) noexcept -> CommandBuffer & {
    if (this != &rval) {
      copy(rval);
      rval.nullify();
    }
    return *this;
  }

  ~CommandBuffer() noexcept {
    //! Add verification pending state !//
    if (device != VK_NULL_HANDLE) {
      vkFreeCommandBuffers(device, cmd_pool, 1, &cmd);
      nullify();
    }
  }

private:
  auto nullify() noexcept -> void {
    device   = VK_NULL_HANDLE;
    queue    = VK_NULL_HANDLE;
    cmd_pool = VK_NULL_HANDLE;
    cmd      = VK_NULL_HANDLE;
  }
  auto copy(const CommandBuffer &other) noexcept -> void {
    this->device   = other.device;
    this->queue    = other.queue;
    this->cmd_pool = other.cmd_pool;
    this->cmd      = other.cmd;
  }
  //== Methods ==//
public:
  auto reset(bool reset_release_ressource = false) {
    return VULKAN_RESULT(vkResetCommandBuffer(
      cmd,
      reset_release_ressource ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT
                              : 0x0));
  }

  enum class BeginCmdUsageBit : uint32_t {
    OneTimeSubmitBit      = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    RenderPassContinueBit = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
    SimultaneousUseBit    = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
  };
  using BeginCmdUsage =
    EnumFlagsWrapper<VkCommandBufferUsageFlags, BeginCmdUsageBit>;
  auto begin(
    BeginCmdUsage usages,
    const VkCommandBufferInheritanceInfo *ptr_inheritance_info = nullptr) {
    VkCommandBufferBeginInfo cmd_begin_info = {
      .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext            = nullptr,
      .flags            = usages.flags,
      .pInheritanceInfo = ptr_inheritance_info,
    };

    return VULKAN_RESULT(vkBeginCommandBuffer(cmd, &cmd_begin_info));
  }

  auto end() { return VULKAN_RESULT(vkEndCommandBuffer(cmd)); }

  auto submit(
    Fence *signal_fence         = nullptr,
    Semaphore *wait_semaphore   = nullptr,
    Semaphore *signal_semaphore = nullptr,
    PipelineStages wait_stages  = PipelineStage::TopOfPipe,
    bool protected_submit             = false) -> VulkanResult<> {

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
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
      .pNext = nullptr,
      .flags = protected_submit ? VK_SUBMIT_PROTECTED_BIT : 0x0u,
      .waitSemaphoreInfoCount   = wait_semaphore ? 1u : 0u,
      .pWaitSemaphoreInfos      = wait_semaphore ? &wait_submit_info : nullptr,
      .commandBufferInfoCount   = 1,
      .pCommandBufferInfos      = &cmd_submit_info,
      .signalSemaphoreInfoCount = signal_semaphore ? 1u : 0u,
      .pSignalSemaphoreInfos = signal_semaphore ? &signal_submit_info : nullptr,
    };

    return VULKAN_RESULT(vkQueueSubmit2(
      queue, 1, &info, signal_fence ? signal_fence->raw() : VK_NULL_HANDLE));
  }

  auto submit(Fence &signal_fence) { return submit(&signal_fence); }
  auto submit(
    Fence &signal_fence,
    Semaphore &wait_semaphore,
    Semaphore &signal_semaphore) {
    return submit(&signal_fence, &wait_semaphore, &signal_semaphore);
  }
};

class Backend;

class ImediateCmd {
  friend Backend;
  //== Attributs ==//
  CommandBuffer cmd;
  Fence fence;
  ///@brief 1 minute
  static constexpr uint64_t DEFAULT_WAIT_TIME = 60'000'000'000;

  //== Constructors ==//
  ImediateCmd(CommandBuffer &&cmd_, Fence &&fence_)
      : cmd(std::move(cmd_)), fence(std::move(fence_)) {
    ASSERT_ERR(
      fence.signaled().unwrap() == false,
      "The fence should be valid and unsignal !");
  }

  //== Methods ==//
public:
  template <typename U>
  auto submit(
    std::function<U(CommandBuffer &cmd)> &&functor,
    uint64_t fence_timeout = DEFAULT_WAIT_TIME) -> VulkanResult<U> {
    using Ret = VulkanResult<U>;

#define RET_IF_ERR(x)                                                          \
  do {                                                                         \
    auto res = x;                                                              \
    if (res.is_err())                                                          \
      return Ret::err(res.unwrap_err());                                       \
  } while (0)

    RET_IF_ERR(fence.reset());
    RET_IF_ERR(cmd.reset());

    RET_IF_ERR(cmd.begin(CommandBuffer::BeginCmdUsageBit::OneTimeSubmitBit));
    auto res = functor(cmd);
    RET_IF_ERR(cmd.end());

    RET_IF_ERR(cmd.submit(fence));
    RET_IF_ERR(fence.wait(fence_timeout));

#undef RET_IF_ERR
    return Ret::ok(std::move(res));
  }

  auto submit(std::function<void(CommandBuffer &cmd)> &&functor) -> VulkanResult<> {
    auto ret_func = [&functor](CommandBuffer &cmd) {
      functor(cmd);
      return VulkanOk{};
    };

    return submit<VulkanOk>(ret_func);
  }
};
}  // namespace vk
}  // namespace mjt