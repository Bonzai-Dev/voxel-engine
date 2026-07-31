function(add_slang_shader_target SHADER_SOURCE OUTPUT_DIRECTORY TARGET)
    get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME_WE)

    set(OUTPUT_FILE "${SHADER_NAME}.spv")

    set(COMPILE_TARGET "spirv")
    set(COMPILE_PROFILE "spirv_1_4")

    add_custom_command(
        OUTPUT ${OUTPUT_DIRECTORY}/${OUTPUT_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIRECTORY}"
        COMMAND $<TARGET_FILE:slangc> ${SHADER_SOURCE}
        -target ${COMPILE_TARGET}
        -profile ${COMPILE_PROFILE}
        -emit-spirv-directly
        -fvk-use-entrypoint-name
        -o ${OUTPUT_DIRECTORY}/${OUTPUT_FILE}

        DEPENDS ${SHADER_SOURCE} slangc
        COMMENT "Compiling shader: ${SHADER_SOURCE}"
        VERBATIM
    )

    add_custom_target(SHADER_${SHADER_NAME} DEPENDS ${OUTPUT_DIRECTORY}/${OUTPUT_FILE})
    add_dependencies(${TARGET} SHADER_${SHADER_NAME})
endfunction()