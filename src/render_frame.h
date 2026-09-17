#ifndef HAMMER_RENDER_FRAME_H
#define HAMMER_RENDER_FRAME_H

#include "hammer_internal.h"
#include "model.h"

/* Runs one iteration of the render loop: acquires a swapchain image,
 * records and submits the wireframe draw commands for `model` (skipped if
 * NULL), presents, and transparently recreates the swapchain when it goes
 * out of date (window resize, minimize, etc). This is runtime frame
 * logic, not Vulkan setup - it belongs to hammer, not to any single
 * vulkan_* configuration module. */
HammerResult hammer_render_draw_frame(HammerContext *context, const HammerModel *model);

#endif /* HAMMER_RENDER_FRAME_H */
