#include "render_frame.h"

#include "camera.h"

#include <stdint.h>

static HammerResult recreate_swapchain(HammerContext *ctx) {
    int width = 0;
    int height = 0;
    hammer_window_get_framebuffer_size(&ctx->window, &width, &height);
    while (width == 0 || height == 0) {
        hammer_window_poll_events();
        hammer_window_get_framebuffer_size(&ctx->window, &width, &height);
    }

    vkDeviceWaitIdle(ctx->device.handle);

    vulkan_present_sync_destroy(ctx->device.handle, &ctx->present_sync);
    vulkan_image_views_destroy(ctx->device.handle, &ctx->image_views);
    vulkan_swapchain_destroy(ctx->device.handle, &ctx->swapchain);

    HammerResult result = vulkan_swapchain_create(
        ctx->physical_device.handle,
        ctx->device.handle,
        ctx->surface.handle,
        &ctx->physical_device.queue_families,
        &ctx->window,
        ctx->enable_vsync,
        &ctx->swapchain
    );
    if (result != HAMMER_SUCCESS) {
        return result;
    }

    result = vulkan_image_views_create(ctx->device.handle, &ctx->swapchain, &ctx->image_views);
    if (result != HAMMER_SUCCESS) {
        return result;
    }

    return vulkan_present_sync_create(ctx->device.handle, ctx->swapchain.image_count, &ctx->present_sync);
}

static void transition_image_layout(
    VkCommandBuffer command_buffer,
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout,
    VkAccessFlags src_access,
    VkAccessFlags dst_access,
    VkPipelineStageFlags src_stage,
    VkPipelineStageFlags dst_stage
) {
    const VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = src_access,
        .dstAccessMask = dst_access,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1,
        },
    };

    vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier);
}

HammerResult hammer_render_draw_frame(HammerContext *ctx, const HammerModel *model) {
    VulkanFrameSync *sync = &ctx->frame_sync[ctx->current_frame];

    vkWaitForFences(ctx->device.handle, 1, &sync->in_flight, VK_TRUE, UINT64_MAX);

    uint32_t image_index = 0;
    const VkResult acquire_result = vkAcquireNextImageKHR(
        ctx->device.handle, ctx->swapchain.handle, UINT64_MAX,
        sync->image_available, VK_NULL_HANDLE, &image_index
    );
    if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
        return recreate_swapchain(ctx);
    }
    if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR) {
        return HAMMER_ERROR_SWAPCHAIN_CREATE_FAILED;
    }

    vkResetFences(ctx->device.handle, 1, &sync->in_flight);

    const VkCommandBuffer command_buffer = ctx->command_pool.buffers[ctx->current_frame];
    vkResetCommandBuffer(command_buffer, 0);

    const VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    vkBeginCommandBuffer(command_buffer, &begin_info);

    const VkImage image = ctx->swapchain.images[image_index];

    transition_image_layout(
        command_buffer, image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    );

    const VkRenderingAttachmentInfo color_attachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = ctx->image_views.views[image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue.color = {{0.01f, 0.01f, 0.02f, 1.0f}},
    };

    const VkRenderingInfo rendering_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {.offset = {0, 0}, .extent = ctx->swapchain.extent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
    };

    vkCmdBeginRendering(command_buffer, &rendering_info);

    if (model != NULL) {
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->pipeline.handle);

        const VkViewport viewport = {
            .width = (float)ctx->swapchain.extent.width,
            .height = (float)ctx->swapchain.extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        const VkRect2D scissor = {.extent = ctx->swapchain.extent};
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        HammerCamera camera_to_use;
        if (ctx->has_custom_camera) {
            camera_to_use = ctx->camera;
        } else {
            hammer_camera_auto_frame(model->bounds_min, model->bounds_max, &camera_to_use);
        }

        const float aspect_ratio = (float)ctx->swapchain.extent.width / (float)ctx->swapchain.extent.height;
        float view_projection[16];
        hammer_camera_build_view_projection(&camera_to_use, aspect_ratio, view_projection);
        vkCmdPushConstants(
            command_buffer, ctx->pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
            sizeof(view_projection), view_projection
        );

        const VkDeviceSize vertex_offset = 0;
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &model->vertex_buffer.handle, &vertex_offset);
        vkCmdBindIndexBuffer(command_buffer, model->index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(command_buffer, model->index_count, 1, 0, 0, 0);
    }

    vkCmdEndRendering(command_buffer);

    transition_image_layout(
        command_buffer, image,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
    );

    vkEndCommandBuffer(command_buffer);

    const VkSemaphore render_finished = ctx->present_sync.render_finished[image_index];

    const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &sync->image_available,
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &render_finished,
    };

    if (vkQueueSubmit(ctx->device.graphics_queue, 1, &submit_info, sync->in_flight) != VK_SUCCESS) {
        return HAMMER_ERROR_SYNC_OBJECT_CREATE_FAILED;
    }

    const VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &render_finished,
        .swapchainCount = 1,
        .pSwapchains = &ctx->swapchain.handle,
        .pImageIndices = &image_index,
    };

    const VkResult present_result = vkQueuePresentKHR(ctx->device.present_queue, &present_info);
    if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR ||
        ctx->window.framebuffer_resized) {
        ctx->window.framebuffer_resized = false;
        const HammerResult recreate_result = recreate_swapchain(ctx);
        if (recreate_result != HAMMER_SUCCESS) {
            return recreate_result;
        }
    } else if (present_result != VK_SUCCESS) {
        return HAMMER_ERROR_SWAPCHAIN_CREATE_FAILED;
    }

    ctx->current_frame = (ctx->current_frame + 1) % HAMMER_MAX_FRAMES_IN_FLIGHT;
    return HAMMER_SUCCESS;
}
