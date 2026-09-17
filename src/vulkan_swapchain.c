#include "vulkan_swapchain.h"

#include <stdint.h>
#include <stdlib.h>

static VkSurfaceFormatKHR choose_surface_format(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, NULL);

    VkSurfaceFormatKHR *formats = malloc(sizeof(VkSurfaceFormatKHR) * format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats);

    VkSurfaceFormatKHR chosen = formats[0];
    for (uint32_t i = 0; i < format_count; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            chosen = formats[i];
            break;
        }
    }

    free(formats);
    return chosen;
}

static VkPresentModeKHR choose_present_mode(VkPhysicalDevice physical_device, VkSurfaceKHR surface, bool enable_vsync) {
    if (enable_vsync) {
        return VK_PRESENT_MODE_FIFO_KHR; /* always supported */
    }

    uint32_t mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &mode_count, NULL);

    VkPresentModeKHR *modes = malloc(sizeof(VkPresentModeKHR) * mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &mode_count, modes);

    VkPresentModeKHR chosen = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < mode_count; i++) {
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            chosen = modes[i];
            break;
        }
    }

    free(modes);
    return chosen;
}

static VkExtent2D choose_extent(const VkSurfaceCapabilitiesKHR *capabilities, const HammerWindow *window) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    }

    int width = 0;
    int height = 0;
    hammer_window_get_framebuffer_size(window, &width, &height);

    VkExtent2D extent = {
        .width = (uint32_t)width,
        .height = (uint32_t)height,
    };

    if (extent.width < capabilities->minImageExtent.width) {
        extent.width = capabilities->minImageExtent.width;
    }
    if (extent.width > capabilities->maxImageExtent.width) {
        extent.width = capabilities->maxImageExtent.width;
    }
    if (extent.height < capabilities->minImageExtent.height) {
        extent.height = capabilities->minImageExtent.height;
    }
    if (extent.height > capabilities->maxImageExtent.height) {
        extent.height = capabilities->maxImageExtent.height;
    }

    return extent;
}

HammerResult vulkan_swapchain_create(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkSurfaceKHR surface,
    const VulkanQueueFamilyIndices *queue_families,
    const HammerWindow *window,
    bool enable_vsync,
    VulkanSwapchain *out_swapchain
) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

    const VkSurfaceFormatKHR surface_format = choose_surface_format(physical_device, surface);
    const VkPresentModeKHR present_mode = choose_present_mode(physical_device, surface, enable_vsync);
    const VkExtent2D extent = choose_extent(&capabilities, window);

    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }

    const uint32_t queue_family_indices[2] = {
        queue_families->graphics_family,
        queue_families->present_family,
    };
    const bool same_family = queue_families->graphics_family == queue_families->present_family;

    const VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = image_count,
        .imageFormat = surface_format.format,
        .imageColorSpace = surface_format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = same_family ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = same_family ? 0u : 2u,
        .pQueueFamilyIndices = same_family ? NULL : queue_family_indices,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(device, &create_info, NULL, &swapchain) != VK_SUCCESS) {
        return HAMMER_ERROR_SWAPCHAIN_CREATE_FAILED;
    }

    uint32_t actual_image_count = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &actual_image_count, NULL);
    VkImage *images = malloc(sizeof(VkImage) * actual_image_count);
    if (images == NULL) {
        vkDestroySwapchainKHR(device, swapchain, NULL);
        return HAMMER_ERROR_OUT_OF_MEMORY;
    }
    vkGetSwapchainImagesKHR(device, swapchain, &actual_image_count, images);

    out_swapchain->handle = swapchain;
    out_swapchain->image_format = surface_format.format;
    out_swapchain->extent = extent;
    out_swapchain->images = images;
    out_swapchain->image_count = actual_image_count;
    return HAMMER_SUCCESS;
}

void vulkan_swapchain_destroy(VkDevice device, VulkanSwapchain *swapchain) {
    free(swapchain->images);
    swapchain->images = NULL;
    swapchain->image_count = 0;

    if (swapchain->handle != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device, swapchain->handle, NULL);
        swapchain->handle = VK_NULL_HANDLE;
    }
}
