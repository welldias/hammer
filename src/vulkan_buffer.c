#include "vulkan_buffer.h"

#include <string.h>

static int32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        const bool type_matches = (type_filter & (1u << i)) != 0;
        const bool properties_match = (memory_properties.memoryTypes[i].propertyFlags & properties) == properties;
        if (type_matches && properties_match) {
            return (int32_t)i;
        }
    }
    return -1;
}

HammerResult vulkan_buffer_create(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memory_properties,
    VulkanBuffer *out_buffer
) {
    const VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VkBuffer buffer = VK_NULL_HANDLE;
    if (vkCreateBuffer(device, &buffer_info, NULL, &buffer) != VK_SUCCESS) {
        return HAMMER_ERROR_BUFFER_CREATE_FAILED;
    }

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(device, buffer, &requirements);

    const int32_t memory_type_index = find_memory_type(physical_device, requirements.memoryTypeBits, memory_properties);
    if (memory_type_index < 0) {
        vkDestroyBuffer(device, buffer, NULL);
        return HAMMER_ERROR_BUFFER_CREATE_FAILED;
    }

    const VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = requirements.size,
        .memoryTypeIndex = (uint32_t)memory_type_index,
    };

    VkDeviceMemory memory = VK_NULL_HANDLE;
    if (vkAllocateMemory(device, &alloc_info, NULL, &memory) != VK_SUCCESS) {
        vkDestroyBuffer(device, buffer, NULL);
        return HAMMER_ERROR_BUFFER_CREATE_FAILED;
    }

    vkBindBufferMemory(device, buffer, memory, 0);

    out_buffer->handle = buffer;
    out_buffer->memory = memory;
    return HAMMER_SUCCESS;
}

void vulkan_buffer_destroy(VkDevice device, VulkanBuffer *buffer) {
    if (buffer->handle != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, buffer->handle, NULL);
        buffer->handle = VK_NULL_HANDLE;
    }
    if (buffer->memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, buffer->memory, NULL);
        buffer->memory = VK_NULL_HANDLE;
    }
}

HammerResult vulkan_buffer_upload_via_staging(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkCommandPool command_pool,
    VkQueue queue,
    const void *data,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VulkanBuffer *out_buffer
) {
    VulkanBuffer staging;
    HammerResult result = vulkan_buffer_create(
        physical_device, device, size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging
    );
    if (result != HAMMER_SUCCESS) {
        return result;
    }

    void *mapped = NULL;
    vkMapMemory(device, staging.memory, 0, size, 0, &mapped);
    memcpy(mapped, data, (size_t)size);
    vkUnmapMemory(device, staging.memory);

    VulkanBuffer device_local;
    result = vulkan_buffer_create(
        physical_device, device, size,
        usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &device_local
    );
    if (result != HAMMER_SUCCESS) {
        vulkan_buffer_destroy(device, &staging);
        return result;
    }

    const VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

    const VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(command_buffer, &begin_info);

    const VkBufferCopy copy_region = {.size = size};
    vkCmdCopyBuffer(command_buffer, staging.handle, device_local.handle, 1, &copy_region);

    vkEndCommandBuffer(command_buffer);

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
    };
    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    vulkan_buffer_destroy(device, &staging);

    *out_buffer = device_local;
    return HAMMER_SUCCESS;
}
