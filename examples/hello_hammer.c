#include <hammer/hammer.h>
#include <stdio.h>

int main(int argc, char **argv) {
    const char *model_path = argc > 1 ? argv[1] : "assets/DamagedHelmet.glb";

    HammerConfig config;
    hammer_config_init_defaults(&config);
    config.app_name = "hello_hammer";
    config.window_title = "hello_hammer";

    HammerContext *ctx = NULL;
    HammerResult result = hammer_context_create(&config, &ctx);
    if (result != HAMMER_SUCCESS) {
        fprintf(stderr, "hammer: failed to create context (error %d)\n", result);
        return 1;
    }

    HammerModel *model = NULL;
    result = hammer_model_load(ctx, model_path, &model);
    if (result != HAMMER_SUCCESS) {
        fprintf(stderr, "hammer: failed to load model '%s' (error %d)\n", model_path, result);
        hammer_context_destroy(ctx);
        return 1;
    }

    while (hammer_context_is_running(ctx)) {
        hammer_context_poll_events(ctx);

        result = hammer_context_draw_frame(ctx, model);
        if (result != HAMMER_SUCCESS) {
            fprintf(stderr, "hammer: failed to draw frame (error %d)\n", result);
            break;
        }
    }

    hammer_model_destroy(ctx, model);
    hammer_context_destroy(ctx);
    return result == HAMMER_SUCCESS ? 0 : 1;
}
