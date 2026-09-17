#include "vulkan_image_views.h"

#include <stdlib.h>

HammerResult vulkan_image_views_create(VkDevice device, const VulkanSwapchain *swapchain, VulkanImageViews *out_views) {
    VkImageView *views = malloc(sizeof(VkImageView) * swapchain->image_count);
    if (views == NULL) {
        return HAMMER_ERROR_OUT_OF_MEMORY;
    }

    for (uint32_t i = 0; i < swapchain->image_count; i++) {
        const VkImageViewCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchain->images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchain->image_format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        if (vkCreateImageView(device, &create_info, NULL, &views[i]) != VK_SUCCESS) {
            for (uint32_t j = 0; j < i; j++) {
                vkDestroyImageView(device, views[j], NULL);
            }
            free(views);
            return HAMMER_ERROR_SWAPCHAIN_CREATE_FAILED;
        }
    }

    out_views->views = views;
    out_views->count = swapchain->image_count;
    return HAMMER_SUCCESS;
}

void vulkan_image_views_destroy(VkDevice device, VulkanImageViews *views) {
    for (uint32_t i = 0; i < views->count; i++) {
        vkDestroyImageView(device, views->views[i], NULL);
    }
    free(views->views);
    views->views = NULL;
    views->count = 0;
}
