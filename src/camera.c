#include "camera.h"

#include <math.h>

#define HAMMER_PI 3.14159265358979323846f

static void vec3_sub(const float a[3], const float b[3], float out[3]) {
    out[0] = a[0] - b[0];
    out[1] = a[1] - b[1];
    out[2] = a[2] - b[2];
}

static float vec3_length(const float v[3]) {
    return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

static void vec3_normalize(float v[3]) {
    const float len = vec3_length(v);
    if (len > 1e-6f) {
        v[0] /= len;
        v[1] /= len;
        v[2] /= len;
    }
}

static void vec3_cross(const float a[3], const float b[3], float out[3]) {
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}

static float vec3_dot(const float a[3], const float b[3]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

/* All matrices here are column-major float[16] (out[col * 4 + row]),
 * matching GLSL's mat4 layout so the result can be pushed as-is. */

static void mat4_look_at(const float eye[3], const float target[3], const float up_hint[3], float out[16]) {
    float forward[3];
    vec3_sub(target, eye, forward);
    vec3_normalize(forward);

    float right[3];
    vec3_cross(forward, up_hint, right);
    vec3_normalize(right);

    float true_up[3];
    vec3_cross(right, forward, true_up);

    out[0] = right[0];
    out[1] = true_up[0];
    out[2] = -forward[0];
    out[3] = 0.0f;

    out[4] = right[1];
    out[5] = true_up[1];
    out[6] = -forward[1];
    out[7] = 0.0f;

    out[8] = right[2];
    out[9] = true_up[2];
    out[10] = -forward[2];
    out[11] = 0.0f;

    out[12] = -vec3_dot(right, eye);
    out[13] = -vec3_dot(true_up, eye);
    out[14] = vec3_dot(forward, eye);
    out[15] = 1.0f;
}

static void mat4_perspective(float fov_y_radians, float aspect_ratio, float near_plane, float far_plane, float out[16]) {
    const float f = 1.0f / tanf(fov_y_radians * 0.5f);

    for (int i = 0; i < 16; i++) {
        out[i] = 0.0f;
    }

    out[0] = f / aspect_ratio;
    out[5] = -f; /* flip Y: Vulkan clip space has +Y pointing down */
    out[10] = far_plane / (near_plane - far_plane);
    out[11] = -1.0f;
    out[14] = (far_plane * near_plane) / (near_plane - far_plane);
}

static void mat4_multiply(const float a[16], const float b[16], float out[16]) {
    float result[16];
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            result[col * 4 + row] = a[0 * 4 + row] * b[col * 4 + 0] + a[1 * 4 + row] * b[col * 4 + 1] +
                                     a[2 * 4 + row] * b[col * 4 + 2] + a[3 * 4 + row] * b[col * 4 + 3];
        }
    }
    for (int i = 0; i < 16; i++) {
        out[i] = result[i];
    }
}

void hammer_camera_build_view_projection(const HammerCamera *camera, float aspect_ratio, float out_matrix[16]) {
    float view[16];
    mat4_look_at(camera->position, camera->target, camera->up, view);

    float projection[16];
    mat4_perspective(camera->fov_y_radians, aspect_ratio, camera->near_plane, camera->far_plane, projection);

    mat4_multiply(projection, view, out_matrix);
}

void hammer_camera_auto_frame(const float bounds_min[3], const float bounds_max[3], HammerCamera *out_camera) {
    const float center[3] = {
        (bounds_min[0] + bounds_max[0]) * 0.5f,
        (bounds_min[1] + bounds_max[1]) * 0.5f,
        (bounds_min[2] + bounds_max[2]) * 0.5f,
    };
    const float extent[3] = {
        bounds_max[0] - bounds_min[0],
        bounds_max[1] - bounds_min[1],
        bounds_max[2] - bounds_min[2],
    };

    float radius = 0.5f * vec3_length(extent);
    if (radius < 1e-4f) {
        radius = 1.0f;
    }

    const float fov_y_radians = 45.0f * (HAMMER_PI / 180.0f);
    const float distance = (radius / sinf(fov_y_radians * 0.5f)) * 1.25f;

    float direction[3] = {1.0f, 0.6f, 1.0f};
    vec3_normalize(direction);

    out_camera->position[0] = center[0] + direction[0] * distance;
    out_camera->position[1] = center[1] + direction[1] * distance;
    out_camera->position[2] = center[2] + direction[2] * distance;
    out_camera->target[0] = center[0];
    out_camera->target[1] = center[1];
    out_camera->target[2] = center[2];
    out_camera->up[0] = 0.0f;
    out_camera->up[1] = 1.0f;
    out_camera->up[2] = 0.0f;
    out_camera->fov_y_radians = fov_y_radians;
    out_camera->near_plane = radius * 0.01f;
    out_camera->far_plane = distance + radius * 2.0f;
}
