#pragma once

#include "backends/vulkan/helpers.hpp"
#define VK_NO_PROTOTYPES
#include <volk/volk.h>

namespace mjt {
namespace vk {

enum class ImageUsageBit {
  TransferSrcBit            = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
  TransferDstBit            = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
  SampledBit                = VK_IMAGE_USAGE_SAMPLED_BIT,
  StorageBit                = VK_IMAGE_USAGE_STORAGE_BIT,
  ColorAttachmentBit        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
  DepthStencilAttachmentBit = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
  TransientAttachmentBit    = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
  InputAttachmentBit        = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
  // Provided by VK_VERSION_1_4
  HostTransferBit = VK_IMAGE_USAGE_HOST_TRANSFER_BIT,
  // Provided by VK_KHR_video_decode_queue
  VideoDecodeDstBitKhr = VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR,
  // Provided by VK_KHR_video_decode_queue
  VideoDecodeSrcBitKhr = VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR,
  // Provided by VK_KHR_video_decode_queue
  VideoDecodeDpbBitKhr = VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR,
  // Provided by VK_EXT_fragment_density_map
  FragmentDensityMapBitExt = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT,
  // Provided by VK_KHR_fragment_shading_rate
  FragmentShadingRateAttachmentBitKhr =
    VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
  // Provided by VK_KHR_video_encode_queue
  VideoEncodeDstBitKhr = VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR,
  // Provided by VK_KHR_video_encode_queue
  VideoEncodeSrcBitKhr = VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR,
  // Provided by VK_KHR_video_encode_queue
  VideoEncodeDpbBitKhr = VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR,
  // Provided by VK_EXT_attachment_feedback_loop_layout
  AttachmentFeedbackLoopBitExt =
    VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT,
  // Provided by VK_HUAWEI_invocation_mask
  InvocationMaskBitHuawei = VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI,
  // Provided by VK_QCOM_image_processing
  SampleWeightBitQcom = VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM,
  // Provided by VK_QCOM_image_processing
  SampleBlockMatchBitQcom = VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM,
  // Provided by VK_ARM_tensors
  TensorAliasingBitArm = VK_IMAGE_USAGE_TENSOR_ALIASING_BIT_ARM,
  // Provided by VK_QCOM_tile_memory_heap
  TileMemoryBitQcom = VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM,
  // Provided by VK_KHR_video_encode_quantization_map
  VideoEncodeQuantizationDeltaMapBitKhr =
    VK_IMAGE_USAGE_VIDEO_ENCODE_QUANTIZATION_DELTA_MAP_BIT_KHR,
  // Provided by VK_KHR_video_encode_quantization_map
  VideoEncodeEmphasisMapBitKhr =
    VK_IMAGE_USAGE_VIDEO_ENCODE_EMPHASIS_MAP_BIT_KHR,
  // Provided by VK_NV_shading_rate_image
  ShadingRateImageBitNv = VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV,
  // Provided by VK_EXT_host_image_copy
  HostTransferBitExt = VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT,
};

using ImageUsage =
  EnumFlagsWrapper<VkImageUsageFlags, ImageUsageBit>;
}
}  // namespace mjt