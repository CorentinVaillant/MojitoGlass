#pragma once
#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"
#include "math/vec2.hpp"

#include "./formats.hpp"
#include "./helpers.hpp"
#include "./images_usages.hpp"
#include "./queue.hpp"
#include "./swapchain.hpp"

namespace mjt {

class VulkanBackend;

enum class SwapchainCreateFlagBit {
  // Provided by VK_VERSION_1_1 with VK_KHR_swapchain, VK_KHR_device_group with
  // VK_KHR_swapchain
  SplitInstanceBindRegionsBitKhr =
    VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR,
  // Provided by VK_VERSION_1_1 with VK_KHR_swapchain
  ProtectedBitKhr = VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR,
  // Provided by VK_KHR_swapchain_mutable_format
  MutableFormatBitKhr = VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR,
  // Provided by VK_EXT_present_timing
  PresentTimingBitExt = VK_SWAPCHAIN_CREATE_PRESENT_TIMING_BIT_EXT,
  // Provided by VK_KHR_present_id2
  PresentId2BitKhr = VK_SWAPCHAIN_CREATE_PRESENT_ID_2_BIT_KHR,
  // Provided by VK_KHR_present_wait2
  PresentWait2BitKhr = VK_SWAPCHAIN_CREATE_PRESENT_WAIT_2_BIT_KHR,
  // Provided by VK_KHR_swapchain_maintenance1
  DeferredMemoryAllocationBitKhr =
    VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_KHR,
  // Provided by VK_EXT_swapchain_maintenance1
  DeferredMemoryAllocationBitExt =
    VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT,
};

using SwapchainCreateFlags =
  EnumFlagsWrapper<VkSwapchainCreateFlagsKHR, SwapchainCreateFlagBit>;

enum class SurfaceTransformBit {
  IdentityBit         = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
  Rotate90Bit         = VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR,
  Rotate180Bit        = VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR,
  Rotate270Bit        = VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR,
  HorizontalMirrorBit = VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR,
  HorizontalMirrorRotate90Bit =
    VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR,
  HorizontalMirrorRotate180Bit =
    VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR,
  HorizontalMirrorRotate270Bit =
    VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR,
  InheritBit = VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR,
};

using SurfaceTransform =
  EnumFlagsWrapper<VkSurfaceTransformFlagsKHR, SurfaceTransformBit>;

enum class CompositeAlphaFlagBits {
  OpaqueBit         = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
  PreMultipliedBit  = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
  PostMultipliedBit = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
  InheritBit        = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
};

using CompositeAlpha =
  EnumFlagsWrapper<VkCompositeAlphaFlagsKHR, CompositeAlphaFlagBits>;

enum class PresentMode {
  Immediate   = VK_PRESENT_MODE_IMMEDIATE_KHR,
  Mailbox     = VK_PRESENT_MODE_MAILBOX_KHR,
  Fifo        = VK_PRESENT_MODE_FIFO_KHR,
  FifoRelaxed = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
  // Provided by VK_KHR_shared_presentable_image
  SharedDemandRefresh = VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
  // Provided by VK_KHR_shared_presentable_image
  SharedContinuousRefresh = VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR,
  // Provided by VK_KHR_present_mode_fifo_latest_ready
  FifoLatestReady = VK_PRESENT_MODE_FIFO_LATEST_READY_KHR,
  // Provided by VK_EXT_present_mode_fifo_latest_ready
  FifoLatestReadyExt = VK_PRESENT_MODE_FIFO_LATEST_READY_EXT,
};

//@brief TODO
class VulkanSwapchainBuilder {
  friend VulkanBackend;

  static constexpr VkSwapchainCreateInfoKHR DEFAULT_CREATE_INFOS{
    .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext                 = nullptr,
    .flags                 = 0x0,
    .surface               = VK_NULL_HANDLE,
    .minImageCount         = 0,
    .imageFormat           = VK_FORMAT_UNDEFINED,
    .imageColorSpace       = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    .imageExtent           = {720, 480},
    .imageArrayLayers      = 1,
    .imageUsage            = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0u,
    .pQueueFamilyIndices   = nullptr,
    .preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
    .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode           = VK_PRESENT_MODE_IMMEDIATE_KHR,
    .clipped               = false,
    .oldSwapchain          = VK_NULL_HANDLE,
  };

  //== Attributs ==//

  VkDevice device                                  = VK_NULL_HANDLE;
  const VkAllocationCallbacks *allocator_callbacks = nullptr;
  std::vector<uint32_t> queue_families             = {};
  VkSwapchainCreateInfoKHR infos                   = DEFAULT_CREATE_INFOS;

  uint32_t surface_caps_min_image_count            = 0;

  //== Constructors ==//
  VulkanSwapchainBuilder(
    VkDevice device_,
    VkSurfaceKHR surface,
    const VkAllocationCallbacks *allocator_callbacks_,
    VkSurfaceCapabilities2KHR &surface_caps,
    std::span<const VkSurfaceFormat2KHR> surface_formats)
      : device(device_), allocator_callbacks(allocator_callbacks_),
        surface_caps_min_image_count(
          surface_caps.surfaceCapabilities.minImageCount) {
    infos.surface         = surface;
    infos.minImageCount   = surface_caps_min_image_count;
    infos.imageFormat     = surface_formats[0].surfaceFormat.format;
    infos.imageColorSpace = surface_formats[0].surfaceFormat.colorSpace;
  }

public:
  NO_COPY(VulkanSwapchainBuilder);
  VulkanSwapchainBuilder(VulkanSwapchainBuilder &&)            = default;
  VulkanSwapchainBuilder &operator=(VulkanSwapchainBuilder &&) = default;

  //== Methods ==//
  auto reset_to_default() -> void {
    VkSurfaceKHR surface = this->infos.surface;
    this->queue_families.clear();
    this->infos         = DEFAULT_CREATE_INFOS;
    this->infos.surface = surface;
  }

  auto set_create_flags(SwapchainCreateFlags flag) -> VulkanSwapchainBuilder & {
    infos.flags = flag.flags;
    return *this;
  }
  auto get_min_image_count() -> uint32_t { return infos.minImageCount; }
  auto set_min_image_count(uint32_t count) -> VulkanSwapchainBuilder & {
    ASSERT_ERR(
      count >= surface_caps_min_image_count,
      "Should be greater than VkSurfaceCapabilitiesKHR::minImageCount ({})",
      surface_caps_min_image_count);
    infos.minImageCount = count;
    return *this;
  }
  auto set_image_format(VulkanFormat format) -> VulkanSwapchainBuilder & {
    infos.imageFormat = static_cast<VkFormat>(format);
    return *this;
  }
  auto set_image_color_space(VulkanColorSpace color_space)
    -> VulkanSwapchainBuilder & {
    infos.imageColorSpace = static_cast<VkColorSpaceKHR>(color_space);
    return *this;
  }
  auto set_image_extent(uint32_t width, uint32_t height)
    -> VulkanSwapchainBuilder & {
    infos.imageExtent.width  = width;
    infos.imageExtent.height = height;
    return *this;
  }
  auto set_image_extent(U32Vec2 size) -> VulkanSwapchainBuilder & {
    infos.imageExtent.width  = size.x;
    infos.imageExtent.height = size.y;
    return *this;
  }
  auto set_image_array_layers(uint32_t layers) -> VulkanSwapchainBuilder & {
    infos.imageArrayLayers = layers;
    return *this;
  }
  auto set_image_usage(VulkanImageUsage usages) -> VulkanSwapchainBuilder & {
    infos.imageUsage = usages.flags;
    return *this;
  }
  auto set_sharing_exclusive() -> VulkanSwapchainBuilder & {
    infos.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    return *this;
  }
  auto set_sharing_concurent() -> VulkanSwapchainBuilder & {
    infos.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    return *this;
  }
  auto add_queue_family(const QueueFamily &q_fam) -> VulkanSwapchainBuilder & {
    queue_families.push_back(q_fam.family_index);
    return *this;
  }
  auto clear_queue_family() -> VulkanSwapchainBuilder & {
    queue_families.clear();
    return *this;
  }
  auto set_surface_transform(SurfaceTransformBit transform)
    -> VulkanSwapchainBuilder & {
    infos.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(transform);
    return *this;
  }
  auto set_composite_alpha(CompositeAlphaFlagBits value)
    -> VulkanSwapchainBuilder & {
    infos.compositeAlpha = static_cast<VkCompositeAlphaFlagBitsKHR>(value);
    return *this;
  }
  auto set_present_mode(PresentMode value) -> VulkanSwapchainBuilder & {
    infos.presentMode = static_cast<VkPresentModeKHR>(value);
    return *this;
  }
  auto set_clipped(bool clipped = true) -> VulkanSwapchainBuilder & {
    infos.clipped = clipped ? VK_TRUE : VK_FALSE;
    return *this;
  }

  auto build() -> VulkanResult<VulkanSwapchain> {
    using Ret                   = VulkanResult<VulkanSwapchain>;

    infos.queueFamilyIndexCount = queue_families.size();
    infos.pQueueFamilyIndices   = queue_families.data();

    VkSwapchainKHR swapchain;

    auto swapchain_result = VULKAN_RESULT(
      vkCreateSwapchainKHR(device, &infos, allocator_callbacks, &swapchain));

    if (swapchain_result.is_err())
      return Ret::err(swapchain_result.unwrap_err());

    uint32_t image_count      = 0;
    auto swapchain_img_result = VULKAN_RESULT(
      vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr));

    if (swapchain_img_result.is_err()) {
      build_fallback(swapchain);
      return Ret::err(swapchain_img_result.unwrap_err());
    }

    std::vector<VkImage> vk_images{image_count, VK_NULL_HANDLE};
    std::vector<VulkanSwapchain::SwapchainImage> images;
    images.reserve(image_count);

    swapchain_img_result = VULKAN_RESULT(vkGetSwapchainImagesKHR(
      device, swapchain, &image_count, vk_images.data()));

    if (swapchain_img_result.is_err()) {
      build_fallback(swapchain);
      return Ret::err(swapchain_img_result.unwrap_err());
    }

    VkImageViewCreateInfo view_infos{
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext    = nullptr,
      .flags    = 0x0u,
      .image    = VK_NULL_HANDLE,  // init after
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format   = infos.imageFormat,
      .components =
        VkComponentMapping{
          VK_COMPONENT_SWIZZLE_R,
          VK_COMPONENT_SWIZZLE_G,
          VK_COMPONENT_SWIZZLE_B,
          VK_COMPONENT_SWIZZLE_A},
      .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    VkSemaphoreCreateInfo semaphore_info{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0x0,
    };

    for (auto &vk_image : vk_images) {
      // create the view
      VkImageView view = VK_NULL_HANDLE;
      view_infos.image = vk_image;
      auto view_result = VULKAN_RESULT(
        vkCreateImageView(device, &view_infos, allocator_callbacks, &view));

      if (view_result.is_err()) {
        build_fallback(swapchain, vk_images, images);
        return Ret::err(view_result.unwrap_err());
      }

      // create the semaphore
      VkSemaphore semaphore = VK_NULL_HANDLE;
      auto semaphore_result = VULKAN_RESULT(vkCreateSemaphore(
        device, &semaphore_info, allocator_callbacks, &semaphore));

      if (semaphore_result.is_err()) {
        images.emplace_back(VK_NULL_HANDLE, view, VK_NULL_HANDLE);
        build_fallback(swapchain, vk_images, images);
        return Ret::err(semaphore_result.unwrap_err());
      }

      images.emplace_back(vk_image, view, semaphore);
      vk_image =
        VK_NULL_HANDLE;  // not to be delete twice by the fallback fuction
    }

    VkSemaphoreCreateInfo image_available_semaphore_info = semaphore_info;
    VkSemaphore image_available                          = VK_NULL_HANDLE;
    auto semaphore_result = VULKAN_RESULT(vkCreateSemaphore(
      device,
      &image_available_semaphore_info,
      allocator_callbacks,
      &image_available));

    if (semaphore_result.is_err()) {
      build_fallback(swapchain, vk_images, images);
      return Ret::err(semaphore_result.unwrap_err());
    }

    VkFenceCreateInfo render_fence_info{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0x0u,
    };
    VkFence render_fence = VK_NULL_HANDLE;
    auto fence_result    = VULKAN_RESULT(vkCreateFence(
      device, &render_fence_info, allocator_callbacks, &render_fence));

    if (fence_result.is_err()) {
      build_fallback(swapchain, vk_images, images, image_available);
      return Ret::err(fence_result.unwrap_err());
    }

    return Ret::ok(VulkanSwapchain(
      device,
      swapchain,
      allocator_callbacks,
      infos,
      std::move(images),
      image_available,
      render_fence));
  }

private:
  auto build_fallback(
    VkSwapchainKHR swapchain,
    [[maybe_unused]] std::span<VkImage> vk_images =
      {},  //> VkImages are destroyed by vkDestroySwapchainKHR
    std::span<VulkanSwapchain::SwapchainImage> images = {},
    VkSemaphore image_available = VK_NULL_HANDLE) -> void {

    if (image_available != VK_NULL_HANDLE)
      vkDestroySemaphore(device, image_available, allocator_callbacks);

    for (auto &image : images) {
      // VkImage &vk_image      = image.image;
      VkImageView &view      = image.view;
      VkSemaphore &semaphore = image.render_finished;

      // if (vk_image != VK_NULL_HANDLE)
      //   vkDestroyImage(device, vk_image, allocator_callbacks);

      if (view != VK_NULL_HANDLE)
        vkDestroyImageView(device, view, allocator_callbacks);

      if (semaphore != VK_NULL_HANDLE)
        vkDestroySemaphore(device, semaphore, allocator_callbacks);
    }

    // for (auto &vk_image : vk_images) {
    //   if (vk_image != VK_NULL_HANDLE)
    //     vkDestroyImage(device, vk_image, allocator_callbacks);
    // }

    vkDestroySwapchainKHR(device, swapchain, allocator_callbacks);
  }
};

}  // namespace mjt
