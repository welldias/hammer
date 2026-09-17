#ifndef HAMMER_VULKAN_LOGICAL_DEVICE_H
#define HAMMER_VULKAN_LOGICAL_DEVICE_H

#include <vulkan/vulkan.h>

#include "hammer/hammer.h"
#include "vulkan_physical_device.h"

typedef struct VulkanLogicalDevice {
    VkDevice handle;
    VkQueue graphics_queue;
    VkQueue present_queue;
} VulkanLogicalDevice;

HammerResult vulkan_logical_device_create(const VulkanPhysicalDevice *physical_device, VulkanLogicalDevice *out_device);
void vulkan_logical_device_destroy(VulkanLogicalDevice *device);

#endif /* HAMMER_VULKAN_LOGICAL_DEVICE_H */
