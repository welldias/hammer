#include "gltf_loader.h"

#include <cgltf.h>

#include <float.h>
#include <stdlib.h>

static void transform_point(const float matrix[16], const float in[3], float out[3]) {
    const float x = in[0];
    const float y = in[1];
    const float z = in[2];

    out[0] = matrix[0] * x + matrix[4] * y + matrix[8] * z + matrix[12];
    out[1] = matrix[1] * x + matrix[5] * y + matrix[9] * z + matrix[13];
    out[2] = matrix[2] * x + matrix[6] * y + matrix[10] * z + matrix[14];
}

static bool append_vertex(HammerRawMesh *mesh, uint32_t *capacity, const float position[3]) {
    if (mesh->vertex_count == *capacity) {
        const uint32_t new_capacity = *capacity == 0 ? 256 : *capacity * 2;
        HammerRawVertex *grown = realloc(mesh->vertices, sizeof(HammerRawVertex) * new_capacity);
        if (grown == NULL) {
            return false;
        }
        mesh->vertices = grown;
        *capacity = new_capacity;
    }

    HammerRawVertex *vertex = &mesh->vertices[mesh->vertex_count++];
    vertex->position[0] = position[0];
    vertex->position[1] = position[1];
    vertex->position[2] = position[2];

    for (int i = 0; i < 3; i++) {
        if (position[i] < mesh->bounds_min[i]) {
            mesh->bounds_min[i] = position[i];
        }
        if (position[i] > mesh->bounds_max[i]) {
            mesh->bounds_max[i] = position[i];
        }
    }

    return true;
}

static bool append_index(HammerRawMesh *mesh, uint32_t *capacity, uint32_t index) {
    if (mesh->index_count == *capacity) {
        const uint32_t new_capacity = *capacity == 0 ? 256 : *capacity * 2;
        uint32_t *grown = realloc(mesh->indices, sizeof(uint32_t) * new_capacity);
        if (grown == NULL) {
            return false;
        }
        mesh->indices = grown;
        *capacity = new_capacity;
    }

    mesh->indices[mesh->index_count++] = index;
    return true;
}

static const cgltf_accessor *find_position_accessor(const cgltf_primitive *primitive) {
    for (cgltf_size i = 0; i < primitive->attributes_count; i++) {
        if (primitive->attributes[i].type == cgltf_attribute_type_position) {
            return primitive->attributes[i].data;
        }
    }
    return NULL;
}

HammerResult gltf_loader_load(const char *path, HammerRawMesh *out_mesh) {
    cgltf_options options = {0};
    cgltf_data *data = NULL;

    if (cgltf_parse_file(&options, path, &data) != cgltf_result_success) {
        return HAMMER_ERROR_MODEL_LOAD_FAILED;
    }

    if (cgltf_load_buffers(&options, data, path) != cgltf_result_success) {
        cgltf_free(data);
        return HAMMER_ERROR_MODEL_LOAD_FAILED;
    }

    *out_mesh = (HammerRawMesh){
        .bounds_min = {FLT_MAX, FLT_MAX, FLT_MAX},
        .bounds_max = {-FLT_MAX, -FLT_MAX, -FLT_MAX},
    };

    uint32_t vertex_capacity = 0;
    uint32_t index_capacity = 0;
    bool allocation_failed = false;

    for (cgltf_size node_index = 0; node_index < data->nodes_count && !allocation_failed; node_index++) {
        const cgltf_node *node = &data->nodes[node_index];
        if (node->mesh == NULL) {
            continue;
        }

        float world_matrix[16];
        cgltf_node_transform_world(node, world_matrix);

        for (cgltf_size prim_index = 0; prim_index < node->mesh->primitives_count; prim_index++) {
            const cgltf_primitive *primitive = &node->mesh->primitives[prim_index];
            if (primitive->type != cgltf_primitive_type_triangles) {
                continue;
            }

            const cgltf_accessor *position_accessor = find_position_accessor(primitive);
            if (position_accessor == NULL) {
                continue;
            }

            const uint32_t base_vertex = out_mesh->vertex_count;

            for (cgltf_size v = 0; v < position_accessor->count; v++) {
                float local_position[3];
                cgltf_accessor_read_float(position_accessor, v, local_position, 3);

                float world_position[3];
                transform_point(world_matrix, local_position, world_position);

                if (!append_vertex(out_mesh, &vertex_capacity, world_position)) {
                    allocation_failed = true;
                    break;
                }
            }
            if (allocation_failed) {
                break;
            }

            if (primitive->indices != NULL) {
                for (cgltf_size i = 0; i < primitive->indices->count; i++) {
                    const uint32_t index = base_vertex + (uint32_t)cgltf_accessor_read_index(primitive->indices, i);
                    if (!append_index(out_mesh, &index_capacity, index)) {
                        allocation_failed = true;
                        break;
                    }
                }
            } else {
                for (cgltf_size i = 0; i < position_accessor->count; i++) {
                    if (!append_index(out_mesh, &index_capacity, base_vertex + (uint32_t)i)) {
                        allocation_failed = true;
                        break;
                    }
                }
            }
            if (allocation_failed) {
                break;
            }
        }
    }

    cgltf_free(data);

    if (allocation_failed) {
        gltf_loader_free(out_mesh);
        return HAMMER_ERROR_OUT_OF_MEMORY;
    }

    if (out_mesh->vertex_count == 0 || out_mesh->index_count == 0) {
        gltf_loader_free(out_mesh);
        return HAMMER_ERROR_MODEL_EMPTY;
    }

    return HAMMER_SUCCESS;
}

void gltf_loader_free(HammerRawMesh *mesh) {
    free(mesh->vertices);
    free(mesh->indices);
    *mesh = (HammerRawMesh){0};
}
