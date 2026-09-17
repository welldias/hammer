#ifndef HAMMER_VULKAN_PHYSICAL_DEVICE_H
#define HAMMER_VULKAN_PHYSICAL_DEVICE_H

#include <vulkan/vulkan.h>

#include "hammer/hammer.h"

typedef struct VulkanQueueFamilyIndices {
    uint32_t graphics_family;
    uint32_t present_family;
} VulkanQueueFamilyIndices;

typedef struct VulkanPhysicalDevice {
    VkPhysicalDevice handle;
    VulkanQueueFamilyIndices queue_families;
} VulkanPhysicalDevice;

/* Picks the first discrete GPU that supports everything hammer needs
 * (Vulkan 1.3 dynamic rendering, swapchain presentation to `surface`,
 * a graphics queue and a present queue), falling back to an integrated
 * GPU if no discrete one qualifies. */
HammerResult vulkan_physical_device_select(VkInstance instance, VkSurfaceKHR surface, VulkanPhysicalDevice *out_device);

#endif /* HAMMER_VULKAN_PHYSICAL_DEVICE_H */
