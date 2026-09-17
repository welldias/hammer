#include "hammer_internal.h"
#include "model.h"
#include "render_frame.h"

#include <stdlib.h>

#define HAMMER_PI 3.14159265358979323846f

void hammer_config_init_defaults(HammerConfig *config) {
    *config = (HammerConfig){
        .app_name      = "Hammer Application",
        .window_title  = "Hammer",
        .window_width  = 1280,
        .window_height = 720,
#ifdef NDEBUG
        .enable_validation = false,
#else
        .enable_validation = true,
#endif
        .enable_vsync = true,
    };
}

HammerResult hammer_context_create(const HammerConfig *config, HammerContext **out_context) {
    HammerContext *ctx = calloc(1, sizeof(HammerContext));
    if (ctx == NULL) {
        return HAMMER_ERROR_OUT_OF_MEMORY;
    }

    ctx->enable_vsync = config->enable_vsync;

    HammerResult result = hammer_window_create(config, &ctx->window);
    if (result != HAMMER_SUCCESS) {
        goto fail_alloc;
    }

    result = vulkan_instance_create(config, &ctx->instance);
    if (result != HAMMER_SUCCESS) {
        goto fail_window;
    }

    result = vulkan_window_surface_create(ctx->instance.handle, &ctx->window, &ctx->surface);
    if (result != HAMMER_SUCCESS) {
        goto fail_instance;
    }

    result = vulkan_physical_device_select(ctx->instance.handle, ctx->surface.handle, &ctx->physical_device);
    if (result != HAMMER_SUCCESS) {
        goto fail_surface;
    }

    result = vulkan_logical_device_create(&ctx->physical_device, &ctx->device);
    if (result != HAMMER_SUCCESS) {
        goto fail_surface; /* physical device selection allocates nothing to undo */
    }

    result = vulkan_swapchain_create(ctx->physical_device.handle, ctx->device.handle, ctx->surface.handle, &ctx->physical_device.queue_families, &ctx->window, ctx->enable_vsync, &ctx->swapchain);
    if (result != HAMMER_SUCCESS) {
        goto fail_logical_device;
    }

    result = vulkan_image_views_create(ctx->device.handle, &ctx->swapchain, &ctx->image_views);
    if (result != HAMMER_SUCCESS) {
        goto fail_swapchain;
    }

    result = vulkan_present_sync_create(ctx->device.handle, ctx->swapchain.image_count, &ctx->present_sync);
    if (result != HAMMER_SUCCESS) {
        goto fail_image_views;
    }

    result = vulkan_pipeline_create(ctx->device.handle, ctx->swapchain.image_format, &ctx->pipeline);
    if (result != HAMMER_SUCCESS) {
        goto fail_present_sync;
    }

    result = vulkan_command_pool_create(ctx->device.handle, ctx->physical_device.queue_families.graphics_family, HAMMER_MAX_FRAMES_IN_FLIGHT, &ctx->command_pool);
    if (result != HAMMER_SUCCESS) {
        goto fail_pipeline;
    }

    uint32_t frames_created = 0;
    for (; frames_created < HAMMER_MAX_FRAMES_IN_FLIGHT; frames_created++) {
        result = vulkan_sync_objects_create(ctx->device.handle, &ctx->frame_sync[frames_created]);
        if (result != HAMMER_SUCCESS) {
            goto fail_command_pool;
        }
    }

    *out_context = ctx;
    return HAMMER_SUCCESS;

fail_command_pool:
    for (uint32_t i = 0; i < frames_created; i++) {
        vulkan_sync_objects_destroy(ctx->device.handle, &ctx->frame_sync[i]);
    }
    vulkan_command_pool_destroy(ctx->device.handle, &ctx->command_pool);
fail_pipeline:
    vulkan_pipeline_destroy(ctx->device.handle, &ctx->pipeline);
fail_present_sync:
    vulkan_present_sync_destroy(ctx->device.handle, &ctx->present_sync);
fail_image_views:
    vulkan_image_views_destroy(ctx->device.handle, &ctx->image_views);
fail_swapchain:
    vulkan_swapchain_destroy(ctx->device.handle, &ctx->swapchain);
fail_logical_device:
    vulkan_logical_device_destroy(&ctx->device);
fail_surface:
    vulkan_window_surface_destroy(ctx->instance.handle, &ctx->surface);
fail_instance:
    vulkan_instance_destroy(&ctx->instance);
fail_window:
    hammer_window_destroy(&ctx->window);
fail_alloc:
    free(ctx);
    return result;
}

void hammer_context_destroy(HammerContext *context) {
    if (context == NULL) {
        return;
    }

    vkDeviceWaitIdle(context->device.handle);

    for (uint32_t i = 0; i < HAMMER_MAX_FRAMES_IN_FLIGHT; i++) {
        vulkan_sync_objects_destroy(context->device.handle, &context->frame_sync[i]);
    }
    vulkan_command_pool_destroy(context->device.handle, &context->command_pool);
    vulkan_pipeline_destroy(context->device.handle, &context->pipeline);
    vulkan_present_sync_destroy(context->device.handle, &context->present_sync);
    vulkan_image_views_destroy(context->device.handle, &context->image_views);
    vulkan_swapchain_destroy(context->device.handle, &context->swapchain);
    vulkan_logical_device_destroy(&context->device);
    vulkan_window_surface_destroy(context->instance.handle, &context->surface);
    vulkan_instance_destroy(&context->instance);
    hammer_window_destroy(&context->window);

    free(context);
}

bool hammer_context_is_running(HammerContext *context) {
    return !hammer_window_should_close(&context->window);
}

void hammer_context_poll_events(HammerContext *context) {
    (void)context;
    hammer_window_poll_events();
}

void hammer_camera_init_defaults(HammerCamera *camera) {
    *camera = (HammerCamera){
        .position = {0.0f, 0.0f, 5.0f},
        .target = {0.0f, 0.0f, 0.0f},
        .up = {0.0f, 1.0f, 0.0f},
        .fov_y_radians = 45.0f * (HAMMER_PI / 180.0f),
        .near_plane = 0.1f,
        .far_plane = 100.0f,
    };
}

void hammer_context_set_camera(HammerContext *context, const HammerCamera *camera) {
    context->camera = *camera;
    context->has_custom_camera = true;
}

HammerResult hammer_model_load(HammerContext *context, const char *path, HammerModel **out_model) {
    return hammer_model_build(
        context->physical_device.handle, context->device.handle,
        context->command_pool.handle, context->device.graphics_queue,
        path, out_model
    );
}

void hammer_model_destroy(HammerContext *context, HammerModel *model) {
    hammer_model_release(context->device.handle, model);
}

HammerResult hammer_context_draw_frame(HammerContext *context, const HammerModel *model) {
    return hammer_render_draw_frame(context, model);
}
