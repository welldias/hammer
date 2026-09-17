#ifndef HAMMER_VULKAN_SYNC_OBJECTS_H
#define HAMMER_VULKAN_SYNC_OBJECTS_H

#include <vulkan/vulkan.h>

#include "hammer/hammer.h"

/* Synchronization objects for a single frame-in-flight slot. Only
 * image_available is here: it is waited on right after being signaled by
 * the *next* acquire for this slot, so reusing it per frame-in-flight is
 * safe. */
typedef struct VulkanFrameSync {
    VkSemaphore image_available;
    VkFence in_flight;
} VulkanFrameSync;

HammerResult vulkan_sync_objects_create(VkDevice device, VulkanFrameSync *out_sync);
void vulkan_sync_objects_destroy(VkDevice device, VulkanFrameSync *sync);

/* render_finished semaphores, one per swapchain image rather than per
 * frame-in-flight: presentation order is not guaranteed to match the
 * frame-in-flight rotation, so a semaphore signaled by a submit and
 * waited on by present must be keyed on the acquired image index (see
 * https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html).
 * Recreated alongside the swapchain since image_count can change. */
typedef struct VulkanPresentSync {
    VkSemaphore *render_finished;
    uint32_t count;
} VulkanPresentSync;

HammerResult vulkan_present_sync_create(VkDevice device, uint32_t image_count, VulkanPresentSync *out_sync);
void vulkan_present_sync_destroy(VkDevice device, VulkanPresentSync *sync);

#endif /* HAMMER_VULKAN_SYNC_OBJECTS_H */
