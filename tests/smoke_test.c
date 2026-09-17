#include <hammer/hammer.h>
#include <stdio.h>

/* Smoke test for the v1 bootstrap: create a context (window + full Vulkan
 * setup), load a tiny glTF model (cgltf's own Box.glb fixture), render a
 * handful of wireframe frames, then tear everything down cleanly.
 * Requires a real display (X11/Wayland) - it does not run headless. */

#define FRAMES_TO_RENDER 5

int main(void) {
    HammerConfig config;
    hammer_config_init_defaults(&config);
    config.app_name = "hammer_smoke_test";
    config.window_title = "hammer_smoke_test";

    HammerContext *ctx = NULL;
    HammerResult result = hammer_context_create(&config, &ctx);
    if (result != HAMMER_SUCCESS) {
        fprintf(stderr, "hammer_context_create failed (error %d)\n", result);
        return 1;
    }

    HammerModel *model = NULL;
    result = hammer_model_load(ctx, HAMMER_SMOKE_TEST_MODEL_PATH, &model);
    if (result != HAMMER_SUCCESS) {
        fprintf(stderr, "hammer_model_load failed (error %d)\n", result);
        hammer_context_destroy(ctx);
        return 1;
    }

    for (int i = 0; i < FRAMES_TO_RENDER && hammer_context_is_running(ctx); i++) {
        hammer_context_poll_events(ctx);

        result = hammer_context_draw_frame(ctx, model);
        if (result != HAMMER_SUCCESS) {
            fprintf(stderr, "hammer_context_draw_frame failed (error %d)\n", result);
            break;
        }
    }

    hammer_model_destroy(ctx, model);
    hammer_context_destroy(ctx);
    return result == HAMMER_SUCCESS ? 0 : 1;
}
