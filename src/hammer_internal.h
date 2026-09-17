#ifndef HAMMER_INTERNAL_H
#define HAMMER_INTERNAL_H

#include "hammer/hammer.h"

#include "vulkan_command_pool.h"
#include "vulkan_image_views.h"
#include "vulkan_instance.h"
#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_pipeline.h"
#include "vulkan_swapchain.h"
#include "vulkan_sync_objects.h"
#include "vulkan_window_surface.h"
#include "window.h"

#define HAMMER_MAX_FRAMES_IN_FLIGHT 2

/* Real definition of the opaque HammerContext declared in hammer.h.
 * Aggregates one struct per subsystem module; hammer never keeps raw
 * Vulkan/GLFW state loose in this struct. */
struct HammerContext {
    bool enable_vsync;

    HammerWindow window;
    VulkanInstance instance;
    VulkanWindowSurface surface;
    VulkanPhysicalDevice physical_device;
    VulkanLogicalDevice device;
    VulkanSwapchain swapchain;
    VulkanImageViews image_views;
    VulkanPipeline pipeline;
    VulkanCommandPool command_pool;
    VulkanFrameSync frame_sync[HAMMER_MAX_FRAMES_IN_FLIGHT];
    VulkanPresentSync present_sync;

    uint32_t current_frame;

    HammerCamera camera;
    bool has_custom_camera;
};

#endif /* HAMMER_INTERNAL_H */
