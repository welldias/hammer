#include "vulkan_sync_objects.h"

#include <stdlib.h>

HammerResult vulkan_sync_objects_create(VkDevice device, VulkanFrameSync *out_sync) {
    const VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    const VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    VkSemaphore image_available = VK_NULL_HANDLE;
    VkFence in_flight = VK_NULL_HANDLE;

    if (vkCreateSemaphore(device, &semaphore_info, NULL, &image_available) != VK_SUCCESS ||
        vkCreateFence(device, &fence_info, NULL, &in_flight) != VK_SUCCESS) {
        if (image_available != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, image_available, NULL);
        }
        if (in_flight != VK_NULL_HANDLE) {
            vkDestroyFence(device, in_flight, NULL);
        }
        return HAMMER_ERROR_SYNC_OBJECT_CREATE_FAILED;
    }

    out_sync->image_available = image_available;
    out_sync->in_flight = in_flight;
    return HAMMER_SUCCESS;
}

void vulkan_sync_objects_destroy(VkDevice device, VulkanFrameSync *sync) {
    if (sync->image_available != VK_NULL_HANDLE) {
        vkDestroySemaphore(device, sync->image_available, NULL);
        sync->image_available = VK_NULL_HANDLE;
    }
    if (sync->in_flight != VK_NULL_HANDLE) {
        vkDestroyFence(device, sync->in_flight, NULL);
        sync->in_flight = VK_NULL_HANDLE;
    }
}

HammerResult vulkan_present_sync_create(VkDevice device, uint32_t image_count, VulkanPresentSync *out_sync) {
    VkSemaphore *semaphores = malloc(sizeof(VkSemaphore) * image_count);
    if (semaphores == NULL) {
        return HAMMER_ERROR_OUT_OF_MEMORY;
    }

    const VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    for (uint32_t i = 0; i < image_count; i++) {
        if (vkCreateSemaphore(device, &semaphore_info, NULL, &semaphores[i]) != VK_SUCCESS) {
            for (uint32_t j = 0; j < i; j++) {
                vkDestroySemaphore(device, semaphores[j], NULL);
            }
            free(semaphores);
            return HAMMER_ERROR_SYNC_OBJECT_CREATE_FAILED;
        }
    }

    out_sync->render_finished = semaphores;
    out_sync->count = image_count;
    return HAMMER_SUCCESS;
}

void vulkan_present_sync_destroy(VkDevice device, VulkanPresentSync *sync) {
    for (uint32_t i = 0; i < sync->count; i++) {
        vkDestroySemaphore(device, sync->render_finished[i], NULL);
    }
    free(sync->render_finished);
    sync->render_finished = NULL;
    sync->count = 0;
}
