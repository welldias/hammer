#ifndef HAMMER_GLTF_LOADER_H
#define HAMMER_GLTF_LOADER_H

#include "hammer/hammer.h"

typedef struct HammerRawVertex {
    float position[3];
} HammerRawVertex;

/* CPU-side geometry extracted from a glTF file: every triangle primitive
 * in the node hierarchy, with node world transforms already applied and
 * merged into one flat vertex/index buffer. */
typedef struct HammerRawMesh {
    HammerRawVertex *vertices;
    uint32_t vertex_count;
    uint32_t *indices;
    uint32_t index_count;
    float bounds_min[3];
    float bounds_max[3];
} HammerRawMesh;

HammerResult gltf_loader_load(const char *path, HammerRawMesh *out_mesh);
void gltf_loader_free(HammerRawMesh *mesh);

#endif /* HAMMER_GLTF_LOADER_H */
