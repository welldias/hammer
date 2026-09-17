#include "vulkan_instance.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const k_validation_layer = "VK_LAYER_KHRONOS_validation";

static bool has_validation_layer_support(void) {
    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    VkLayerProperties *layers = malloc(sizeof(VkLayerProperties) * layer_count);
    if (layers == NULL) {
        return false;
    }
    vkEnumerateInstanceLayerProperties(&layer_count, layers);

    bool found = false;
    for (uint32_t i = 0; i < layer_count; i++) {
        if (strcmp(layers[i].layerName, k_validation_layer) == 0) {
            found = true;
            break;
        }
    }

    free(layers);
    return found;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
    void *user_data
) {
    (void)type;
    (void)user_data;

    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        fprintf(stderr, "[hammer][vulkan] %s\n", callback_data->pMessage);
    }
    return VK_FALSE;
}

static void fill_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info) {
    *create_info = (VkDebugUtilsMessengerCreateInfoEXT){
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_callback,
    };
}

HammerResult vulkan_instance_create(const HammerConfig *config, VulkanInstance *out_instance) {
    const bool want_validation = config->enable_validation;
    if (want_validation && !has_validation_layer_support()) {
        return HAMMER_ERROR_VALIDATION_LAYER_UNAVAILABLE;
    }

    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    uint32_t extension_count = glfw_extension_count + (want_validation ? 1u : 0u);
    const char **extensions = malloc(sizeof(const char *) * extension_count);
    if (extensions == NULL) {
        return HAMMER_ERROR_OUT_OF_MEMORY;
    }
    memcpy(extensions, glfw_extensions, sizeof(const char *) * glfw_extension_count);
    if (want_validation) {
        extensions[glfw_extension_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    const VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = config->app_name,
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .pEngineName = "hammer",
        .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3,
    };

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    fill_debug_messenger_create_info(&debug_create_info);

    const VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = want_validation ? &debug_create_info : NULL,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = want_validation ? 1u : 0u,
        .ppEnabledLayerNames = want_validation ? &k_validation_layer : NULL,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = extensions,
    };

    VkInstance instance = VK_NULL_HANDLE;
    const VkResult result = vkCreateInstance(&create_info, NULL, &instance);
    free(extensions);
    if (result != VK_SUCCESS) {
        return HAMMER_ERROR_INSTANCE_CREATE_FAILED;
    }

    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
    if (want_validation) {
        PFN_vkCreateDebugUtilsMessengerEXT create_messenger =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (create_messenger == NULL ||
            create_messenger(instance, &debug_create_info, NULL, &debug_messenger) != VK_SUCCESS) {
            vkDestroyInstance(instance, NULL);
            return HAMMER_ERROR_INSTANCE_CREATE_FAILED;
        }
    }

    out_instance->handle = instance;
    out_instance->debug_messenger = debug_messenger;
    return HAMMER_SUCCESS;
}

void vulkan_instance_destroy(VulkanInstance *instance) {
    if (instance->debug_messenger != VK_NULL_HANDLE) {
        PFN_vkDestroyDebugUtilsMessengerEXT destroy_messenger =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance->handle, "vkDestroyDebugUtilsMessengerEXT");
        if (destroy_messenger != NULL) {
            destroy_messenger(instance->handle, instance->debug_messenger, NULL);
        }
        instance->debug_messenger = VK_NULL_HANDLE;
    }

    if (instance->handle != VK_NULL_HANDLE) {
        vkDestroyInstance(instance->handle, NULL);
        instance->handle = VK_NULL_HANDLE;
    }
}
