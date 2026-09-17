#include "vulkan_command_pool.h"

#include <stdlib.h>

HammerResult vulkan_command_pool_create(VkDevice device, uint32_t queue_family, uint32_t buffer_count, VulkanCommandPool *out_pool) {
    const VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family,
    };

    VkCommandPool pool = VK_NULL_HANDLE;
    if (vkCreateCommandPool(device, &pool_info, NULL, &pool) != VK_SUCCESS) {
        return HAMMER_ERROR_COMMAND_POOL_CREATE_FAILED;
    }

    VkCommandBuffer *buffers = malloc(sizeof(VkCommandBuffer) * buffer_count);
    if (buffers == NULL) {
        vkDestroyCommandPool(device, pool, NULL);
        return HAMMER_ERROR_OUT_OF_MEMORY;
    }

    const VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = buffer_count,
    };

    if (vkAllocateCommandBuffers(device, &alloc_info, buffers) != VK_SUCCESS) {
        free(buffers);
        vkDestroyCommandPool(device, pool, NULL);
        return HAMMER_ERROR_COMMAND_POOL_CREATE_FAILED;
    }

    out_pool->handle = pool;
    out_pool->buffers = buffers;
    out_pool->buffer_count = buffer_count;
    return HAMMER_SUCCESS;
}

void vulkan_command_pool_destroy(VkDevice device, VulkanCommandPool *pool) {
    free(pool->buffers);
    pool->buffers = NULL;
    pool->buffer_count = 0;

    if (pool->handle != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, pool->handle, NULL);
        pool->handle = VK_NULL_HANDLE;
    }
}
