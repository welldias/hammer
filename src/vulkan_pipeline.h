#ifndef HAMMER_VULKAN_PIPELINE_H
#define HAMMER_VULKAN_PIPELINE_H

#include <vulkan/vulkan.h>

#include "hammer/hammer.h"

typedef struct VulkanPipeline {
    VkPipelineLayout layout;
    VkPipeline handle;
} VulkanPipeline;

/* Builds the graphics pipeline for hammer's built-in demo triangle, using
 * Vulkan 1.3 dynamic rendering (no VkRenderPass/VkFramebuffer). */
HammerResult vulkan_pipeline_create(VkDevice device, VkFormat color_attachment_format, VulkanPipeline *out_pipeline);
void vulkan_pipeline_destroy(VkDevice device, VulkanPipeline *pipeline);

#endif /* HAMMER_VULKAN_PIPELINE_H */
