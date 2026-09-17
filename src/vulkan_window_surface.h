#ifndef HAMMER_VULKAN_WINDOW_SURFACE_H
#define HAMMER_VULKAN_WINDOW_SURFACE_H

#include <vulkan/vulkan.h>

#include "hammer/hammer.h"
#include "window.h"

typedef struct VulkanWindowSurface {
    VkSurfaceKHR handle;
} VulkanWindowSurface;

/* The only place GLFW and Vulkan touch directly (glfwCreateWindowSurface). */
HammerResult vulkan_window_surface_create(VkInstance instance, const HammerWindow *window, VulkanWindowSurface *out_surface);
void vulkan_window_surface_destroy(VkInstance instance, VulkanWindowSurface *surface);

#endif /* HAMMER_VULKAN_WINDOW_SURFACE_H */
