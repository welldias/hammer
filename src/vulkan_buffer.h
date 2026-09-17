#ifndef HAMMER_VULKAN_BUFFER_H
#define HAMMER_VULKAN_BUFFER_H

#include <vulkan/vulkan.h>

#include "hammer/hammer.h"

typedef struct VulkanBuffer {
    VkBuffer handle;
    VkDeviceMemory memory;
} VulkanBuffer;

HammerResult vulkan_buffer_create(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memory_properties,
    VulkanBuffer *out_buffer
);
void vulkan_buffer_destroy(VkDevice device, VulkanBuffer *buffer);

/* Uploads `size` bytes from `data` into a new device-local buffer (usage
 * gains VK_BUFFER_USAGE_TRANSFER_DST_BIT automatically), via a temporary
 * host-visible staging buffer and a one-shot command buffer submitted to
 * `queue` from `command_pool`. Blocks until the upload completes. */
HammerResult vulkan_buffer_upload_via_staging(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkCommandPool command_pool,
    VkQueue queue,
    const void *data,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VulkanBuffer *out_buffer
);

#endif /* HAMMER_VULKAN_BUFFER_H */
