#ifndef HAMMER_WINDOW_H
#define HAMMER_WINDOW_H

#include <stdbool.h>

#include "hammer/hammer.h"

typedef struct GLFWwindow GLFWwindow;

/* Thin wrapper around a GLFW window. Contains no Vulkan calls at all -
 * the bridge to Vulkan (VkSurfaceKHR creation) lives in
 * vulkan_window_surface.c. */
typedef struct HammerWindow {
    GLFWwindow *handle;
    bool framebuffer_resized;
} HammerWindow;

HammerResult hammer_window_create(const HammerConfig *config, HammerWindow *out_window);
void hammer_window_destroy(HammerWindow *window);
bool hammer_window_should_close(const HammerWindow *window);
void hammer_window_poll_events(void);
void hammer_window_get_framebuffer_size(const HammerWindow *window, int *width, int *height);

#endif /* HAMMER_WINDOW_H */
