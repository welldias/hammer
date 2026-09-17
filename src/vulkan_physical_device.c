#include "vulkan_physical_device.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static const char *const k_required_device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
#define REQUIRED_DEVICE_EXTENSION_COUNT \
    (sizeof(k_required_device_extensions) / sizeof(k_required_device_extensions[0]))

static bool supports_required_extensions(VkPhysicalDevice device) {
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);
    if (extension_count == 0) {
        return false;
    }

    VkExtensionProperties *available = malloc(sizeof(VkExtensionProperties) * extension_count);
    if (available == NULL) {
        return false;
    }
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, available);

    bool all_found = true;
    for (size_t i = 0; i < REQUIRED_DEVICE_EXTENSION_COUNT; i++) {
        bool found = false;
        for (uint32_t j = 0; j < extension_count; j++) {
            if (strcmp(available[j].extensionName, k_required_device_extensions[i]) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            all_found = false;
            break;
        }
    }

    free(available);
    return all_found;
}

static bool supports_wireframe_fill_mode(VkPhysicalDevice device) {
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    return features.fillModeNonSolid == VK_TRUE;
}

static bool supports_dynamic_rendering(VkPhysicalDevice device) {
    VkPhysicalDeviceVulkan13Features vulkan13_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
    };
    VkPhysicalDeviceFeatures2 features2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &vulkan13_features,
    };
    vkGetPhysicalDeviceFeatures2(device, &features2);
    return vulkan13_features.dynamicRendering == VK_TRUE;
}

static bool find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface, VulkanQueueFamilyIndices *out_indices) {
    uint32_t family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, NULL);
    if (family_count == 0) {
        return false;
    }

    VkQueueFamilyProperties *families = malloc(sizeof(VkQueueFamilyProperties) * family_count);
    if (families == NULL) {
        return false;
    }
    vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, families);

    bool has_graphics = false;
    bool has_present = false;
    uint32_t graphics_family = 0;
    uint32_t present_family = 0;

    for (uint32_t i = 0; i < family_count; i++) {
        if (!has_graphics && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            graphics_family = i;
            has_graphics = true;
        }

        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (!has_present && present_support == VK_TRUE) {
            present_family = i;
            has_present = true;
        }

        if (has_graphics && has_present) {
            break;
        }
    }

    free(families);

    if (!has_graphics || !has_present) {
        return false;
    }

    out_indices->graphics_family = graphics_family;
    out_indices->present_family = present_family;
    return true;
}

static bool has_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, NULL);

    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL);

    return format_count > 0 && present_mode_count > 0;
}

static bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface, VulkanQueueFamilyIndices *out_indices) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    if (properties.apiVersion < VK_API_VERSION_1_3) {
        return false;
    }

    if (!supports_required_extensions(device) || !supports_dynamic_rendering(device) ||
        !supports_wireframe_fill_mode(device) || !has_swapchain_support(device, surface)) {
        return false;
    }

    return find_queue_families(device, surface, out_indices);
}

HammerResult vulkan_physical_device_select(VkInstance instance, VkSurfaceKHR surface, VulkanPhysicalDevice *out_device) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, NULL);
    if (device_count == 0) {
        return HAMMER_ERROR_NO_SUITABLE_GPU;
    }

    VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * device_count);
    if (devices == NULL) {
        return HAMMER_ERROR_OUT_OF_MEMORY;
    }
    vkEnumeratePhysicalDevices(instance, &device_count, devices);

    VkPhysicalDevice best_device = VK_NULL_HANDLE;
    VulkanQueueFamilyIndices best_indices = {0};
    bool best_is_discrete = false;

    for (uint32_t i = 0; i < device_count; i++) {
        VulkanQueueFamilyIndices indices;
        if (!is_device_suitable(devices[i], surface, &indices)) {
            continue;
        }

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(devices[i], &properties);
        const bool is_discrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

        if (best_device == VK_NULL_HANDLE || (is_discrete && !best_is_discrete)) {
            best_device = devices[i];
            best_indices = indices;
            best_is_discrete = is_discrete;
        }
    }

    free(devices);

    if (best_device == VK_NULL_HANDLE) {
        return HAMMER_ERROR_NO_SUITABLE_GPU;
    }

    out_device->handle = best_device;
    out_device->queue_families = best_indices;
    return HAMMER_SUCCESS;
}
