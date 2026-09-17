#ifndef HAMMER_CAMERA_H
#define HAMMER_CAMERA_H

#include "hammer/hammer.h"

/* Builds a combined view*projection matrix (column-major, ready to upload
 * as-is in a mat4 push constant) from a camera and an aspect ratio. */
void hammer_camera_build_view_projection(const HammerCamera *camera, float aspect_ratio, float out_matrix[16]);

/* Computes a camera that frames [bounds_min, bounds_max] entirely, viewed
 * from a fixed elevated angle. Used when the developer hasn't set a
 * custom camera via hammer_context_set_camera(). */
void hammer_camera_auto_frame(const float bounds_min[3], const float bounds_max[3], HammerCamera *out_camera);

#endif /* HAMMER_CAMERA_H */
