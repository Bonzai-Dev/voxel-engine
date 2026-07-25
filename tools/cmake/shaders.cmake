function(add_slang_shader_target SHADER_SOURCE OUTPUT_DIRECTORY)
    get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME_WE)

    set(OUTPUT_FILE "${SHADER_NAME}.spv")

    set(COMPILE_TARGET "spirv")
    set(COMPILE_PROFILE "spirv_1_4")

    add_custom_command(
        OUTPUT ${OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIRECTORY}"
        COMMAND $<TARGET_FILE:slangc> ${SHADER_SOURCE}
            -target ${COMPILE_TARGET}
            -profile ${COMPILE_PROFILE}
            -emit-spirv-directly
            -fvk-use-entrypoint-name
            -o ${OUTPUT_DIRECTORY}/${OUTPUT_FILE}

        DEPENDS ${SHADER_SOURCE} slangc
        COMMENT "Compiling Slang shader"
        VERBATIM
    )

    add_custom_target(SHADER_${SHADER_NAME} DEPENDS ${OUTPUT_DIRECTORY})
    add_dependencies(PROJECT_SHADERS SHADER_${SHADER_NAME})
endfunction()