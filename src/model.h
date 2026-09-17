#ifndef HAMMER_MODEL_H
#define HAMMER_MODEL_H

#include <vulkan/vulkan.h>

#include "hammer/hammer.h"
#include "vulkan_buffer.h"

/* Real definition of the opaque HammerModel declared in hammer.h. */
struct HammerModel {
    VulkanBuffer vertex_buffer;
    VulkanBuffer index_buffer;
    uint32_t index_count;
    float bounds_min[3];
    float bounds_max[3];
};

/* Parses `path` with the glTF loader and uploads the resulting geometry
 * to the GPU. Named differently from the public hammer_model_load() -
 * that one is implemented in context.c, per hammer's rule that only
 * context.c implements the public API. */
HammerResult hammer_model_build(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkCommandPool command_pool,
    VkQueue queue,
    const char *path,
    HammerModel **out_model
);
void hammer_model_release(VkDevice device, HammerModel *model);

#endif /* HAMMER_MODEL_H */
