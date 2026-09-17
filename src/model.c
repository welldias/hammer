#include "model.h"

#include "gltf_loader.h"

#include <stdlib.h>
#include <string.h>

HammerResult hammer_model_build(
    VkPhysicalDevice physical_device,
    VkDevice device,
    VkCommandPool command_pool,
    VkQueue queue,
    const char *path,
    HammerModel **out_model
) {
    HammerRawMesh raw_mesh;
    HammerResult result = gltf_loader_load(path, &raw_mesh);
    if (result != HAMMER_SUCCESS) {
        return result;
    }

    HammerModel *model = calloc(1, sizeof(HammerModel));
    if (model == NULL) {
        gltf_loader_free(&raw_mesh);
        return HAMMER_ERROR_OUT_OF_MEMORY;
    }

    result = vulkan_buffer_upload_via_staging(
        physical_device, device, command_pool, queue,
        raw_mesh.vertices, sizeof(HammerRawVertex) * raw_mesh.vertex_count,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        &model->vertex_buffer
    );
    if (result != HAMMER_SUCCESS) {
        goto fail_vertex_buffer;
    }

    result = vulkan_buffer_upload_via_staging(
        physical_device, device, command_pool, queue,
        raw_mesh.indices, sizeof(uint32_t) * raw_mesh.index_count,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        &model->index_buffer
    );
    if (result != HAMMER_SUCCESS) {
        goto fail_index_buffer;
    }

    model->index_count = raw_mesh.index_count;
    memcpy(model->bounds_min, raw_mesh.bounds_min, sizeof(model->bounds_min));
    memcpy(model->bounds_max, raw_mesh.bounds_max, sizeof(model->bounds_max));

    gltf_loader_free(&raw_mesh);
    *out_model = model;
    return HAMMER_SUCCESS;

fail_index_buffer:
    vulkan_buffer_destroy(device, &model->vertex_buffer);
fail_vertex_buffer:
    gltf_loader_free(&raw_mesh);
    free(model);
    return result;
}

void hammer_model_release(VkDevice device, HammerModel *model) {
    if (model == NULL) {
        return;
    }

    vulkan_buffer_destroy(device, &model->index_buffer);
    vulkan_buffer_destroy(device, &model->vertex_buffer);
    free(model);
}
