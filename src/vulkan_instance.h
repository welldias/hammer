#ifndef HAMMER_VULKAN_INSTANCE_H
#define HAMMER_VULKAN_INSTANCE_H

#include <vulkan/vulkan.h>

#include "hammer/hammer.h"

/* The VkInstance and, when validation is enabled, the debug messenger that
 * forwards Vulkan validation layer messages to stderr. */
typedef struct VulkanInstance {
    VkInstance handle;
    VkDebugUtilsMessengerEXT debug_messenger; /* VK_NULL_HANDLE when validation is disabled */
} VulkanInstance;

HammerResult vulkan_instance_create(const HammerConfig *config, VulkanInstance *out_instance);
void vulkan_instance_destroy(VulkanInstance *instance);

#endif /* HAMMER_VULKAN_INSTANCE_H */
