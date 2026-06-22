#pragma once

#include "backends/vulkan/helpers.hpp"

#include <vulkan/vulkan_core.h>
#define VK_NO_PROTOTYPES
#include "common.hpp"
#include "vma_usages.hpp"

#include <volk/volk.h>

namespace mjt {

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
  TransferSrc         = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  TransferDst         = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
  UniformTexelBuffer  = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
  StorageTexelBuffer  = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
  UniformBuffer       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  StorageBuffer       = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
  IndexBuffer         = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
  VertexBuffer        = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
  IndirectBuffer      = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
  ShaderDeviceAddress = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
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

///@brief Wrapper for a VkBuffer.
///@important `T` must be a trivially copyable type
template <GpuUploadable T> class VulkanBuffer {
  // == Attributs == //

  VmaAllocator allocator       = VK_NULL_HANDLE;
  VkBuffer buffer              = VK_NULL_HANDLE;
  VmaAllocation allocation     = VK_NULL_HANDLE;
  VmaAllocationInfo alloc_info = {};
  size_t count;

  // == Constructors == //
  VulkanBuffer(
    VmaAllocator allocator_,
    VkBuffer buf,
    VmaAllocation alloc,
    VmaAllocationInfo allocation_info,
    size_t n)
      : allocator(allocator_), buffer(buf), allocation(alloc),
        alloc_info(allocation_info), count(n) {}

public:
  NO_COPY(VulkanBuffer);

  static auto create(
    VmaAllocator allocator,
    const VkBufferCreateInfo *ptr_buffer_info,
    const VmaAllocationCreateInfo *ptr_alloc_create_info,
    size_t count) -> VulkanResult<VulkanBuffer> {
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;

    return VULKAN_RESULT(vmaCreateBuffer(
                           allocator,
                           ptr_buffer_info,
                           ptr_alloc_create_info,
                           &buffer,
                           &allocation,
                           &allocation_info))
      .replace_ok(
        VulkanBuffer(allocator, buffer, allocation, allocation_info, count));
  }

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

public:
  ///@brief write from CPU memory to the buffer.
  ///@important if not created with AllocationCreateBits::Mapped and
  /// VulkanBufferUsageBit::TransferDst, will fail and crash.
  auto write(size_t dst_offset, std::span<const T> src) -> void {
    ASSERT_ERR(allocator != VK_NULL_HANDLE, "write on a destroyed buffer.");
    ASSERT_ERR(
      alloc_info.pMappedData != nullptr, "alloc_info.pMappedData is null.");
    ASSERT_ERR(dst_offset < count, "dst_offset >= count.")
    memcpy(
      reinterpret_cast<uint8_t *>(alloc_info.pMappedData)
        + dst_offset * sizeof(T),
      reinterpret_cast<const uint8_t *>(src.data()),
      std::min((count - dst_offset) * sizeof(T), src.size_bytes()));
  }
  ///@brief write from CPU memory to the buffer.
  ///@important if not created with AllocationCreateBits::Mapped and
  /// VulkanBufferUsageBit::TransferDst, will fail and crash.
  inline auto write(size_t dst_offset, size_t src_count, const T *src) {
    write(dst_offset, {src, src_count});
  }
  ///@brief write from CPU memory to the buffer.
  ///@important if not created with AllocationCreateBits::Mapped and
  /// VulkanBufferUsageBit::TransferDst, will fail and crash.
  inline auto write(std::span<const T> src) { write(0, src); }

  ///@brief flush the memory using vmaFlushAllocation.
  auto flush() -> VulkanResult<> {
    return VULKAN_RESULT(
      vmaFlushAllocation(allocator, allocation, 0, alloc_info.size));
  }
  ///@brief invalidate the memory using vmaInvalidateAllocation.
  auto invalidate() -> VulkanResult<> {
    return VULKAN_RESULT(
      vmaInvalidateAllocation(allocator, allocation, 0, alloc_info.size));
  }

  ///@brief read from the buffer to the CPU memory.
  ///@important if not created with AllocationCreateBits::Mapped and
  /// VulkanBufferUsageBit::TransferSrc, will fail and crash.
  auto read(size_t src_offset, std::span<T> dst) -> void {
    ASSERT_ERR(allocator != VK_NULL_HANDLE, "read on a destroyed buffer.");
    ASSERT_ERR(
      alloc_info.pMappedData != nullptr, "alloc_info.pMappedData is null.");
    ASSERT_ERR(src_offset < count, "src_offset >= count.")
    memcpy(
      reinterpret_cast<uint8_t *>(dst.data()),
      reinterpret_cast<const uint8_t *>(alloc_info.pMappedData)
        + src_offset * sizeof(T),
      std::min((count - src_offset) * sizeof(T), dst.size_bytes()));
  }

  ///@brief read from the buffer to the CPU memory.
  ///@important if not created with AllocationCreateBits::Mapped and
  /// VulkanBufferUsageBit::TransferSrc, will fail and crash.
  inline auto read(size_t src_offset, size_t dst_count, T *dst) {
    read(src_offset, {dst, dst_count});
  }
  ///@brief read from the buffer to the CPU memory.
  ///@important if not created with AllocationCreateBits::Mapped and
  /// VulkanBufferUsageBit::TransferSrc, will fail and crash.
  inline auto read(std::span<T> dst) { read(0, dst); }
};

// ===== VulkanMemoryAllocator =====

///@brief Wrapper for a vulkan memory allocator
class VulkanMemoryAllocator {
  // == Attributs == //
  VmaAllocator allocator = VK_NULL_HANDLE;

  // == Constructors == //
  VulkanMemoryAllocator() = delete;
  VulkanMemoryAllocator(VmaAllocator allocator_) : allocator(allocator_) {}

public:
  static auto create(VmaAllocatorCreateInfo &infos)
    -> Result<VulkanMemoryAllocator, VulkanError> {
    VmaAllocator allocator;
    return VULKAN_RESULT(vmaCreateAllocator(&infos, &allocator))
      .replace_ok(VulkanMemoryAllocator(allocator));
  }

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
  ///@brief create a `VulkanBuffer<T>`
  ///@param[in] count : the number of ellement of type `T` to be in the buffer
  ///@param[in] usage : vulkan buffer usage
  ///@param[in] mem_usage : VMA memory usage
  ///@param[in] alloc_create_flag : creation flag for the allocation
  ///@param[in] sharing_mode_queues : to put if the buffer is shared accros
  /// multiple thread, with each queue being in each thread
  template <GpuUploadable T>
  auto create_buffer(
    size_t count,
    VulkanBufferUsage usage,
    MemoryUsage mem_usage                   = MemoryUsage::AutoPreferDevice,
    AllocationCreateFlags alloc_create_flag = {},
    std::optional<std::span<uint32_t>> sharing_mode_queues = std::nullopt)
    -> VulkanResult<VulkanBuffer<T>> {

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

    VmaAllocationCreateInfo alloc_create_info{
      .flags          = alloc_create_flag.flags,
      .usage          = static_cast<VmaMemoryUsage>(mem_usage),
      .requiredFlags  = 0x0,
      .preferredFlags = 0x0,
      .memoryTypeBits = 0x0,
      .pool           = nullptr,
      .pUserData      = nullptr,
      .priority       = 0,
    };

    return VulkanBuffer<T>::create(
      allocator, &buffer_info, &alloc_create_info, count);
  }
};

}  // namespace mjt