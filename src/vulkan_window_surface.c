#include "vulkan_window_surface.h"

#include <GLFW/glfw3.h>

HammerResult vulkan_window_surface_create(VkInstance instance, const HammerWindow *window, VulkanWindowSurface *out_surface) {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (glfwCreateWindowSurface(instance, window->handle, NULL, &surface) != VK_SUCCESS) {
        return HAMMER_ERROR_SURFACE_CREATE_FAILED;
    }

    out_surface->handle = surface;
    return HAMMER_SUCCESS;
}

void vulkan_window_surface_destroy(VkInstance instance, VulkanWindowSurface *surface) {
    if (surface->handle != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance, surface->handle, NULL);
        surface->handle = VK_NULL_HANDLE;
    }
}
