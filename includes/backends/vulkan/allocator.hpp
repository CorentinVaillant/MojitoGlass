#pragma once

#include "backends/vulkan/helpers.hpp"

#include <vulkan/vulkan_core.h>
#define VK_NO_PROTOTYPES
#include "common.hpp"
#include "vma_usages.hpp"

#include <volk/volk.h>

// Forward declarations :
class VulkanBackend;

// ===== Flags =====
enum class AllocatorCreateBit : uint32_t {
  ExternallySynchronized  = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
  DedicatedAllocation     = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT,
  BindMemory2             = VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT,
  MemoryBudget            = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
  AmdDeviceCoherentMemory = VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT,
  BufferDeviceAddress     = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
  MemoryPriority          = VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT,
  Maintenance4            = VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT,
  Maintenance5            = VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT,
  ExternalMemoryWin32     = VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT,
};

using AllocatorCreateFlags =
  EnumFlagsWrapper<VmaAllocatorCreateFlags, AllocatorCreateBit>;

enum class VulkanBufferUsageBit : uint32_t {
  TransferSrcBit         = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  TransferDstBit         = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
  UniformTexelBufferBit  = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
  StorageTexelBufferBit  = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
  UniformBufferBit       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  StorageBufferBit       = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
  IndexBufferBit         = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
  VertexBufferBit        = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
  IndirectBufferBit      = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
  ShaderDeviceAddressBit = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
};

using VulkanBufferUsage =
  EnumFlagsWrapper<VkBufferUsageFlags, VulkanBufferUsageBit>;

///@brief Only non deprecated usages of VmaMemoryUsage
enum class MemoryUsage : uint32_t {
  Unknown            = VMA_MEMORY_USAGE_UNKNOWN,
  GpuLazilyAllocated = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED,
  Auto               = VMA_MEMORY_USAGE_AUTO,
  AutoPreferDevice   = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
  AutoPreferHost     = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
};

enum class AllocationCreateBits : uint32_t {
  DedicatedMemory    = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
  NeverAllocate      = VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT,
  Mapped             = VMA_ALLOCATION_CREATE_MAPPED_BIT,
  UserDataCopyString = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT,
  UpperAddress       = VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT,
  DontBind           = VMA_ALLOCATION_CREATE_DONT_BIND_BIT,
  WithinBudget       = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT,
  CanAlias           = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT,
  HostAccessSequentialWrite =
    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
  HostAccessRandom = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
  HostAccessAllowTransferInstead =
    VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT,
  StrategyMinMemory = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT,
  StrategyMinTime   = VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT,
  StrategyMinOffset = VMA_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT
};

using AllocationCreateFlags =
  EnumFlagsWrapper<VmaAllocationCreateFlags, AllocationCreateBits>;

class VulkanMemoryAllocator;

// Buffer
template <GpuUploadable T> class VulkanBuffer {
  friend VulkanMemoryAllocator;
  // == Attributs == //

  VmaAllocator      allocator  = VK_NULL_HANDLE;
  VkBuffer          buffer     = VK_NULL_HANDLE;
  VmaAllocation     allocation = VK_NULL_HANDLE;
  VmaAllocationInfo alloc_info = {};
  size_t            count;

  // == Constructors == //
  VulkanBuffer(
    VmaAllocator      allocator_,
    VkBuffer          buf,
    VmaAllocation     alloc,
    VmaAllocationInfo allocation_info,
    size_t            n)
      : allocator(allocator_), buffer(buf), allocation(alloc),
        alloc_info(allocation_info), count(n) {}

public:
  VulkanBuffer(VulkanBuffer &&rval) noexcept {
    copy(rval);
    rval.nullify();
  }

  auto operator=(VulkanBuffer &&rval) noexcept -> VulkanBuffer & {
    if (this != &rval) {
      copy(rval);
      rval.nullify();
    }
    return *this;
  }

  ~VulkanBuffer() noexcept {
    if (allocator != VK_NULL_HANDLE) {
      vmaDestroyBuffer(allocator, buffer, allocation);
      nullify();
    }
  }

private:
  void nullify() noexcept {
    allocator  = VK_NULL_HANDLE;
    buffer     = VK_NULL_HANDLE;
    allocation = VK_NULL_HANDLE;
    alloc_info = {};
    count      = 0;
  }

  // == Methods == //

private:
  auto copy(const VulkanBuffer &other) noexcept -> void {
    allocator  = other.allocator;
    buffer     = other.buffer;
    allocation = other.allocation;
    alloc_info = other.alloc_info;
    count      = other.count;
  }
};

// ===== VulkanMemoryAllocator =====
class VulkanMemoryAllocator {
  friend VulkanBackend;

  // == Attributs == //
  VmaAllocator allocator = VK_NULL_HANDLE;

  // == Constructors == //
  VulkanMemoryAllocator() = delete;
  VulkanMemoryAllocator(VmaAllocatorCreateInfo &infos) {
    VK_CHECK(vmaCreateAllocator(&infos, &allocator));
  }

public:
  VulkanMemoryAllocator(VulkanMemoryAllocator &&rval) noexcept {
    this->allocator = rval.allocator;
    rval.nullify();
  }

  VulkanMemoryAllocator &operator=(VulkanMemoryAllocator &&rval) noexcept {
    if (this != &rval) {
      this->allocator = rval.allocator;
      rval.nullify();
    }
    return *this;
  }

  ~VulkanMemoryAllocator() noexcept {
    if (allocator)
      vmaDestroyAllocator(allocator);
    nullify();
  }

private:
  inline void nullify() noexcept { allocator = VK_NULL_HANDLE; }

  // == Methods ==
public:
  template <GpuUploadable T>
  auto create_buffer(
    size_t                count,
    VulkanBufferUsage     usage,
    MemoryUsage           mem_usage         = MemoryUsage::AutoPreferDevice,
    AllocationCreateFlags alloc_create_flag = {},
    std::optional<std::span<uint32_t>> sharing_mode_queues = std::nullopt)
    -> VulkanBuffer<T> {

    VkBufferCreateInfo buffer_info{
      .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext       = nullptr,
      .flags       = 0,  // ~
      .size        = sizeof(T) * count,
      .usage       = usage.flags,
      .sharingMode = sharing_mode_queues.has_value()
                       ? VK_SHARING_MODE_CONCURRENT
                       : VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount =
        sharing_mode_queues.has_value()
          ? static_cast<uint32_t>(sharing_mode_queues->size())
          : 0u,
      .pQueueFamilyIndices =
        sharing_mode_queues.has_value() ? sharing_mode_queues->data() : nullptr,

    };

    VmaAllocationCreateInfo alloc_info{
      .flags          = alloc_create_flag.flags,
      .usage          = static_cast<VmaMemoryUsage>(mem_usage),
      .requiredFlags  = 0x0,
      .preferredFlags = 0x0,
      .memoryTypeBits = 0x0,
      .pool           = nullptr,
      .pUserData      = nullptr,
      .priority       = 0,
    };

    VkBuffer          buffer;
    VmaAllocation     allocation;
    VmaAllocationInfo allocation_info;
    VK_CHECK(vmaCreateBuffer(
      allocator,
      &buffer_info,
      &alloc_info,
      &buffer,
      &allocation,
      &allocation_info));

    return VulkanBuffer<T>(
      allocator, buffer, allocation, allocation_info, count);
  }
};