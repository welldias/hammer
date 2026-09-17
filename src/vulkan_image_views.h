#ifndef HAMMER_VULKAN_IMAGE_VIEWS_H
#define HAMMER_VULKAN_IMAGE_VIEWS_H

#include <vulkan/vulkan.h>

#include "hammer/hammer.h"
#include "vulkan_swapchain.h"

typedef struct VulkanImageViews {
    VkImageView *views;
    uint32_t count;
} VulkanImageViews;

HammerResult vulkan_image_views_create(VkDevice device, const VulkanSwapchain *swapchain, VulkanImageViews *out_views);
void vulkan_image_views_destroy(VkDevice device, VulkanImageViews *views);

#endif /* HAMMER_VULKAN_IMAGE_VIEWS_H */
