# hammer_add_embedded_shader(<out-header-var> <glsl-source> <c-symbol>)
#
# Compiles a GLSL shader to SPIR-V via glslc, then embeds the resulting
# bytes into a generated C header (see EmbedShader.cmake) exposing them as
# `<c-symbol>` / `<c-symbol>_size`. Sets <out-header-var> in the parent
# scope to the generated header's path.
function(hammer_add_embedded_shader OUT_HEADER GLSL_SOURCE SYMBOL)
    get_filename_component(shader_name "${GLSL_SOURCE}" NAME)
    set(spv_file "${CMAKE_CURRENT_BINARY_DIR}/${shader_name}.spv")
    set(header_file "${CMAKE_CURRENT_BINARY_DIR}/${shader_name}.spv.h")

    add_custom_command(
        OUTPUT "${spv_file}"
        COMMAND Vulkan::glslc "${GLSL_SOURCE}" -o "${spv_file}"
        DEPENDS "${GLSL_SOURCE}"
        COMMENT "Compiling ${shader_name} to SPIR-V"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${header_file}"
        COMMAND ${CMAKE_COMMAND}
                -DINPUT_FILE=${spv_file}
                -DOUTPUT_FILE=${header_file}
                -DSYMBOL=${SYMBOL}
                -P "${CMAKE_SOURCE_DIR}/cmake/EmbedShader.cmake"
        DEPENDS "${spv_file}" "${CMAKE_SOURCE_DIR}/cmake/EmbedShader.cmake"
        COMMENT "Embedding ${shader_name}.spv into ${SYMBOL}"
        VERBATIM
    )

    set(${OUT_HEADER} "${header_file}" PARENT_SCOPE)
endfunction()
