#include "vulkan_logical_device.h"

HammerResult vulkan_logical_device_create(const VulkanPhysicalDevice *physical_device, VulkanLogicalDevice *out_device) {
    const uint32_t graphics_family = physical_device->queue_families.graphics_family;
    const uint32_t present_family = physical_device->queue_families.present_family;
    const bool same_family = graphics_family == present_family;

    const float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_infos[2];
    uint32_t queue_create_info_count = 0;

    queue_create_infos[queue_create_info_count++] = (VkDeviceQueueCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = graphics_family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };
    if (!same_family) {
        queue_create_infos[queue_create_info_count++] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = present_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };
    }

    VkPhysicalDeviceVulkan13Features vulkan13_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .dynamicRendering = VK_TRUE,
    };

    static const char *const k_device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    const VkPhysicalDeviceFeatures enabled_features = {
        .fillModeNonSolid = VK_TRUE, /* required for wireframe (VK_POLYGON_MODE_LINE) */
    };

    const VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &vulkan13_features,
        .queueCreateInfoCount = queue_create_info_count,
        .pQueueCreateInfos = queue_create_infos,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = k_device_extensions,
        .pEnabledFeatures = &enabled_features,
    };

    VkDevice device = VK_NULL_HANDLE;
    if (vkCreateDevice(physical_device->handle, &create_info, NULL, &device) != VK_SUCCESS) {
        return HAMMER_ERROR_DEVICE_CREATE_FAILED;
    }

    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue present_queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(device, graphics_family, 0, &graphics_queue);
    vkGetDeviceQueue(device, present_family, 0, &present_queue);

    out_device->handle = device;
    out_device->graphics_queue = graphics_queue;
    out_device->present_queue = present_queue;
    return HAMMER_SUCCESS;
}

void vulkan_logical_device_destroy(VulkanLogicalDevice *device) {
    if (device->handle != VK_NULL_HANDLE) {
        vkDestroyDevice(device->handle, NULL);
        device->handle = VK_NULL_HANDLE;
    }
}
