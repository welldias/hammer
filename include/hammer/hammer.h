#ifndef HAMMER_H
#define HAMMER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HammerContext HammerContext;
typedef struct HammerModel HammerModel;

typedef enum HammerResult {
    HAMMER_SUCCESS = 0,
    HAMMER_ERROR_WINDOW_INIT_FAILED,
    HAMMER_ERROR_WINDOW_CREATE_FAILED,
    HAMMER_ERROR_INSTANCE_CREATE_FAILED,
    HAMMER_ERROR_VALIDATION_LAYER_UNAVAILABLE,
    HAMMER_ERROR_SURFACE_CREATE_FAILED,
    HAMMER_ERROR_NO_SUITABLE_GPU,
    HAMMER_ERROR_DEVICE_CREATE_FAILED,
    HAMMER_ERROR_SWAPCHAIN_CREATE_FAILED,
    HAMMER_ERROR_PIPELINE_CREATE_FAILED,
    HAMMER_ERROR_COMMAND_POOL_CREATE_FAILED,
    HAMMER_ERROR_SYNC_OBJECT_CREATE_FAILED,
    HAMMER_ERROR_BUFFER_CREATE_FAILED,
    HAMMER_ERROR_MODEL_LOAD_FAILED,
    HAMMER_ERROR_MODEL_EMPTY,
    HAMMER_ERROR_OUT_OF_MEMORY,
} HammerResult;

/* Public configuration for a hammer context. Always call
 * hammer_config_init_defaults() before filling in fields: it guarantees
 * every field has a sane value, including ones added in future versions. */
typedef struct HammerConfig {
    const char *app_name;
    const char *window_title;
    uint32_t    window_width;
    uint32_t    window_height;
    bool        enable_validation;
    bool        enable_vsync;
} HammerConfig;

void hammer_config_init_defaults(HammerConfig *config);

/* Creates a window and initializes Vulkan (instance, device, swapchain,
 * pipeline, sync objects) according to config. On failure, *out_context is
 * left untouched and no cleanup is required by the caller. */
HammerResult hammer_context_create(const HammerConfig *config, HammerContext **out_context);

/* Destroys the context. Every HammerModel loaded from it must be
 * destroyed with hammer_model_destroy() BEFORE calling this. */
void hammer_context_destroy(HammerContext *context);

/* Returns false once the user has requested to close the window. */
bool hammer_context_is_running(HammerContext *context);

/* Pumps the windowing system's event queue. Call once per loop iteration. */
void hammer_context_poll_events(HammerContext *context);

/* A camera is plain data - keep as many as you like (e.g. one per view)
 * and pass whichever should be active to hammer_context_set_camera(). */
typedef struct HammerCamera {
    float position[3];
    float target[3];
    float up[3];
    float fov_y_radians;
    float near_plane;
    float far_plane;
} HammerCamera;

void hammer_camera_init_defaults(HammerCamera *camera);

/* Sets the active camera used by hammer_context_draw_frame(). Until this
 * is called, hammer automatically frames whatever model is being drawn
 * (computes a camera from its bounding box), so a default project can
 * render something reasonable without the developer writing any camera
 * math at all. */
void hammer_context_set_camera(HammerContext *context, const HammerCamera *camera);

/* Loads a glTF/GLB file (via cgltf) and uploads its geometry to the GPU.
 * Walks the full node hierarchy, applies each node's world transform, and
 * merges every triangle primitive it finds into one drawable mesh. Only
 * vertex positions are used in this version - no materials, textures,
 * skinning or animation. Must be called after hammer_context_create(). */
HammerResult hammer_model_load(HammerContext *context, const char *path, HammerModel **out_model);
void hammer_model_destroy(HammerContext *context, HammerModel *model);

/* Renders one frame. `model` may be NULL to just clear and present an
 * empty frame. Handles image acquisition, submission, presentation and
 * swapchain recreation (e.g. on window resize) internally. */
HammerResult hammer_context_draw_frame(HammerContext *context, const HammerModel *model);

#ifdef __cplusplus
}
#endif

#endif /* HAMMER_H */
