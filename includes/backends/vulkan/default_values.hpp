#pragma once

#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"

namespace mjt {

// Features
constexpr VkPhysicalDeviceVulkan11Features DEFAULT_PHYSICAL_DEVICE_11_FEATURES{
  .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
  .pNext = nullptr,
  .storageBuffer16BitAccess           = VK_FALSE,
  .uniformAndStorageBuffer16BitAccess = VK_FALSE,
  .storagePushConstant16              = VK_FALSE,
  .storageInputOutput16               = VK_FALSE,
  .multiview                          = VK_FALSE,
  .multiviewGeometryShader            = VK_FALSE,
  .multiviewTessellationShader        = VK_FALSE,
  .variablePointersStorageBuffer      = VK_FALSE,
  .variablePointers                   = VK_FALSE,
  .protectedMemory                    = VK_FALSE,
  .samplerYcbcrConversion             = VK_FALSE,
  .shaderDrawParameters               = VK_FALSE,

};
constexpr VkPhysicalDeviceVulkan12Features DEFAULT_PHYSICAL_DEVICE_12_FEATURES{
  .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
  .pNext = nullptr,
  .samplerMirrorClampToEdge                           = VK_FALSE,
  .drawIndirectCount                                  = VK_FALSE,
  .storageBuffer8BitAccess                            = VK_FALSE,
  .uniformAndStorageBuffer8BitAccess                  = VK_FALSE,
  .storagePushConstant8                               = VK_FALSE,
  .shaderBufferInt64Atomics                           = VK_FALSE,
  .shaderSharedInt64Atomics                           = VK_FALSE,
  .shaderFloat16                                      = VK_FALSE,
  .shaderInt8                                         = VK_FALSE,
  .descriptorIndexing                                 = VK_FALSE,
  .shaderInputAttachmentArrayDynamicIndexing          = VK_FALSE,
  .shaderUniformTexelBufferArrayDynamicIndexing       = VK_FALSE,
  .shaderStorageTexelBufferArrayDynamicIndexing       = VK_FALSE,
  .shaderUniformBufferArrayNonUniformIndexing         = VK_FALSE,
  .shaderSampledImageArrayNonUniformIndexing          = VK_FALSE,
  .shaderStorageBufferArrayNonUniformIndexing         = VK_FALSE,
  .shaderStorageImageArrayNonUniformIndexing          = VK_FALSE,
  .shaderInputAttachmentArrayNonUniformIndexing       = VK_FALSE,
  .shaderUniformTexelBufferArrayNonUniformIndexing    = VK_FALSE,
  .shaderStorageTexelBufferArrayNonUniformIndexing    = VK_FALSE,
  .descriptorBindingUniformBufferUpdateAfterBind      = VK_FALSE,
  .descriptorBindingSampledImageUpdateAfterBind       = VK_FALSE,
  .descriptorBindingStorageImageUpdateAfterBind       = VK_FALSE,
  .descriptorBindingStorageBufferUpdateAfterBind      = VK_FALSE,
  .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE,
  .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE,
  .descriptorBindingUpdateUnusedWhilePending          = VK_FALSE,
  .descriptorBindingPartiallyBound                    = VK_FALSE,
  .descriptorBindingVariableDescriptorCount           = VK_FALSE,
  .runtimeDescriptorArray                             = VK_FALSE,
  .samplerFilterMinmax                                = VK_FALSE,
  .scalarBlockLayout                                  = VK_FALSE,
  .imagelessFramebuffer                               = VK_FALSE,
  .uniformBufferStandardLayout                        = VK_FALSE,
  .shaderSubgroupExtendedTypes                        = VK_FALSE,
  .separateDepthStencilLayouts                        = VK_FALSE,
  .hostQueryReset                                     = VK_FALSE,
  .timelineSemaphore                                  = VK_FALSE,
  .bufferDeviceAddress                                = VK_FALSE,
  .bufferDeviceAddressCaptureReplay                   = VK_FALSE,
  .bufferDeviceAddressMultiDevice                     = VK_FALSE,
  .vulkanMemoryModel                                  = VK_FALSE,
  .vulkanMemoryModelDeviceScope                       = VK_FALSE,
  .vulkanMemoryModelAvailabilityVisibilityChains      = VK_FALSE,
  .shaderOutputViewportIndex                          = VK_FALSE,
  .shaderOutputLayer                                  = VK_FALSE,
  .subgroupBroadcastDynamicId                         = VK_FALSE,
};
constexpr VkPhysicalDeviceVulkan13Features DEFAULT_PHYSICAL_DEVICE_13_FEATURES{
  .sType              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
  .pNext              = nullptr,
  .robustImageAccess  = VK_FALSE,
  .inlineUniformBlock = VK_FALSE,
  .descriptorBindingInlineUniformBlockUpdateAfterBind = VK_FALSE,
  .pipelineCreationCacheControl                       = VK_FALSE,
  .privateData                                        = VK_FALSE,
  .shaderDemoteToHelperInvocation                     = VK_FALSE,
  .shaderTerminateInvocation                          = VK_FALSE,
  .subgroupSizeControl                                = VK_FALSE,
  .computeFullSubgroups                               = VK_FALSE,
  .synchronization2                                   = VK_FALSE,
  .textureCompressionASTC_HDR                         = VK_FALSE,
  .shaderZeroInitializeWorkgroupMemory                = VK_FALSE,
  .dynamicRendering                                   = VK_FALSE,
  .shaderIntegerDotProduct                            = VK_FALSE,
  .maintenance4                                       = VK_FALSE,
};
constexpr VkPhysicalDeviceVulkan14Features DEFAULT_PHYSICAL_DEVICE_14_FEATURES{
  .sType                = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
  .pNext                = nullptr,
  .globalPriorityQuery  = VK_FALSE,
  .shaderSubgroupRotate = VK_FALSE,
  .shaderSubgroupRotateClustered          = VK_FALSE,
  .shaderFloatControls2                   = VK_FALSE,
  .shaderExpectAssume                     = VK_FALSE,
  .rectangularLines                       = VK_FALSE,
  .bresenhamLines                         = VK_FALSE,
  .smoothLines                            = VK_FALSE,
  .stippledRectangularLines               = VK_FALSE,
  .stippledBresenhamLines                 = VK_FALSE,
  .stippledSmoothLines                    = VK_FALSE,
  .vertexAttributeInstanceRateDivisor     = VK_FALSE,
  .vertexAttributeInstanceRateZeroDivisor = VK_FALSE,
  .indexTypeUint8                         = VK_FALSE,
  .dynamicRenderingLocalRead              = VK_FALSE,
  .maintenance5                           = VK_FALSE,
  .maintenance6                           = VK_FALSE,
  .pipelineProtectedAccess                = VK_FALSE,
  .pipelineRobustness                     = VK_FALSE,
  .hostImageCopy                          = VK_FALSE,
  .pushDescriptor                         = VK_FALSE,
};

// Extensions
constexpr std::initializer_list<const char *> DEFAULT_EXTENSIONS{
  VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME};

}  // namespace mjt