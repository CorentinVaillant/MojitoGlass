#pragma once

#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "common.hpp"

// Features
constexpr VkPhysicalDeviceVulkan11Features DEFAULT_PHYSICAL_DEVICE_11_FEATURES{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
};
constexpr VkPhysicalDeviceVulkan12Features DEFAULT_PHYSICAL_DEVICE_12_FEATURES{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
};
constexpr VkPhysicalDeviceVulkan13Features DEFAULT_PHYSICAL_DEVICE_13_FEATURES{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
};
constexpr VkPhysicalDeviceVulkan14Features DEFAULT_PHYSICAL_DEVICE_14_FEATURES{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
};

// Extensions
constexpr std::initializer_list<const char*> DEFAULT_EXTENSIONS {
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
};