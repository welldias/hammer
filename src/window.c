#include "window.h"

#include <GLFW/glfw3.h>

static void framebuffer_resize_callback(GLFWwindow *handle, int width, int height) {
    (void)width;
    (void)height;
    HammerWindow *window = (HammerWindow *)glfwGetWindowUserPointer(handle);
    window->framebuffer_resized = true;
}

HammerResult hammer_window_create(const HammerConfig *config, HammerWindow *out_window) {
    if (glfwInit() == GLFW_FALSE) {
        return HAMMER_ERROR_WINDOW_INIT_FAILED;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow *handle = glfwCreateWindow(
        (int)config->window_width,
        (int)config->window_height,
        config->window_title,
        NULL,
        NULL
    );
    if (handle == NULL) {
        glfwTerminate();
        return HAMMER_ERROR_WINDOW_CREATE_FAILED;
    }

    out_window->handle = handle;
    out_window->framebuffer_resized = false;

    glfwSetWindowUserPointer(handle, out_window);
    glfwSetFramebufferSizeCallback(handle, framebuffer_resize_callback);

    return HAMMER_SUCCESS;
}

void hammer_window_destroy(HammerWindow *window) {
    if (window->handle != NULL) {
        glfwDestroyWindow(window->handle);
        window->handle = NULL;
    }
    glfwTerminate();
}

bool hammer_window_should_close(const HammerWindow *window) {
    return glfwWindowShouldClose(window->handle) != 0;
}

void hammer_window_poll_events(void) {
    glfwPollEvents();
}

void hammer_window_get_framebuffer_size(const HammerWindow *window, int *width, int *height) {
    glfwGetFramebufferSize(window->handle, width, height);
}
