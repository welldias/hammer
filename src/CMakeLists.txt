include(${CMAKE_SOURCE_DIR}/cmake/HammerShaders.cmake)

hammer_add_embedded_shader(WIREFRAME_VERT_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/shaders/wireframe.vert" wireframe_vert_spv)
hammer_add_embedded_shader(WIREFRAME_FRAG_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/shaders/wireframe.frag" wireframe_frag_spv)

add_custom_target(hammer_shaders DEPENDS "${WIREFRAME_VERT_HEADER}" "${WIREFRAME_FRAG_HEADER}")

# cgltf is vendored third-party code: compiled in its own object library,
# deliberately not linked against hammer_warnings, so its own warnings
# never break hammer's -Werror build.
add_library(hammer_cgltf OBJECT third_party_cgltf.c)
target_include_directories(hammer_cgltf PRIVATE ${CMAKE_SOURCE_DIR}/external/cgltf)
set_target_properties(hammer_cgltf PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(HAMMER_SOURCES
    context.c
    render_frame.c
    window.c
    camera.c
    gltf_loader.c
    model.c
    vulkan_buffer.c
    vulkan_command_pool.c
    vulkan_image_views.c
    vulkan_instance.c
    vulkan_logical_device.c
    vulkan_physical_device.c
    vulkan_pipeline.c
    vulkan_swapchain.c
    vulkan_sync_objects.c
    vulkan_window_surface.c
)

add_library(hammer_static STATIC ${HAMMER_SOURCES} $<TARGET_OBJECTS:hammer_cgltf>)
add_library(hammer_shared SHARED ${HAMMER_SOURCES} $<TARGET_OBJECTS:hammer_cgltf>)

foreach(hammer_target hammer_static hammer_shared)
    target_include_directories(${hammer_target}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}
            ${CMAKE_SOURCE_DIR}/external/cgltf
    )
    target_link_libraries(${hammer_target}
        PUBLIC
            Vulkan::Vulkan
        PRIVATE
            glfw
            hammer_warnings
    )
    if(NOT MSVC)
        target_link_libraries(${hammer_target} PRIVATE m)
    endif()
    add_dependencies(${hammer_target} hammer_shaders)
    set_target_properties(${hammer_target} PROPERTIES OUTPUT_NAME hammer)
endforeach()

set_target_properties(hammer_shared PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

add_library(hammer::hammer ALIAS hammer_static)
