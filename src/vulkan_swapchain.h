#ifndef HAMMER_VULKAN_SWAPCHAIN_H
#define HAMMER_VULKAN_SWAPCHAIN_H

#include <vulkan/vulkan.h>

#include "hammer/hammer.h"
#include "vulkan_physical_device.h"
#include "window.h"

typedef struct VulkanSwapchain {
    VkSwapchainKHR handle;
    VkFormat image_format;
    VkExtent2D extent;
    VkImage *images;
    uint32_t image_count;
} VulkanSwapchain;

HammerResult vulkan_swapchain_create(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkSurfaceKHR surface,
    const VulkanQueueFamilyIndices *queue_families,
    const HammerWindow *window,
    bool enable_vsync,
    VulkanSwapchain *out_swapchain
);
void vulkan_swapchain_destroy(VkDevice device, VulkanSwapchain *swapchain);

#endif /* HAMMER_VULKAN_SWAPCHAIN_H */
