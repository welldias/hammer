#ifndef HAMMER_VULKAN_COMMAND_POOL_H
#define HAMMER_VULKAN_COMMAND_POOL_H

#include <vulkan/vulkan.h>

#include "hammer/hammer.h"

typedef struct VulkanCommandPool {
    VkCommandPool handle;
    VkCommandBuffer *buffers; /* buffer_count entries, one per frame-in-flight */
    uint32_t buffer_count;
} VulkanCommandPool;

HammerResult vulkan_command_pool_create(VkDevice device, uint32_t queue_family, uint32_t buffer_count, VulkanCommandPool *out_pool);
void vulkan_command_pool_destroy(VkDevice device, VulkanCommandPool *pool);

#endif /* HAMMER_VULKAN_COMMAND_POOL_H */
