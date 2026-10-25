file(
    GLOB_RECURSE EVENTS_SOURCES CONFIGURE_DEPENDS
    "src/*.h" "src/*.c" "src/*.hpp" "src/*.cpp" "src/*.h"
)

# Creating core library
#add_library(engine STATIC ${APPLICATION_SOURCES} ${EVENTS_SOURCES} ${RHI_SOURCES})
add_library(engine STATIC ${APPLICATION_SOURCES} ${EVENTS_SOURCES})
add_library(voxel-engine::engine ALIAS engine)
target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(engine PRIVATE ${RHI_ENABLED_BACKENDS})
target_compile_definitions(
    engine PRIVATE
    $<$<CONFIG:Debug>:ENGINE_DEBUG>
    $<$<CONFIG:Release>:ENGINE_RELEASE>
)

if(WIN32)
    target_compile_definitions(engine PRIVATE ENGINE_PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(engine PRIVATE ENGINE_PLATFORM_LINUX)
elseif(APPLE)
    target_compile_definitions(engine PRIVATE ENGINE_PLATFORM_APPLE)
endif()

if (${MSVC})
    target_compile_definitions(engine PRIVATE ENGINE_COMPILER_MSVC)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_compile_definitions(engine PRIVATE ENGINE_COMPILER_GCC)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang") # TODO: validate
    target_compile_definitions(engine PRIVATE ENGINE_COMPILER_CLANG)
endif()

add_subdirectory(thirdparty)

add_custom_command(
    OUTPUT "${SHADERS_DIRECTORY}/triangle.vert.spv"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADERS_DIRECTORY}"
    COMMAND slangc "${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/triangle.vert.slang"
    -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name
    -o "${SHADERS_DIRECTORY}/triangle.vert.spv"
    DEPENDS slang-glslang
    COMMENT "Compiling triangle vertex shader"
    VERBATIM
)
add_custom_command(
    OUTPUT "${SHADERS_DIRECTORY}/triangle.frag.spv"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADERS_DIRECTORY}"
    COMMAND slangc "${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/triangle.frag.slang"
    -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name
    -o "${SHADERS_DIRECTORY}/triangle.frag.spv"
    DEPENDS slang-glslang
    COMMENT "Compiling triangle fragment shader"
    VERBATIM
)
add_custom_target(engine-triangle-shaders
    DEPENDS
    "${SHADERS_DIRECTORY}/triangle.vert.spv"
    "${SHADERS_DIRECTORY}/triangle.frag.spv"
)
add_dependencies(engine engine-triangle-shaders)
target_compile_definitions(
    engine PRIVATE
    ENGINE_TRIANGLE_VERTEX_SHADER="${SHADERS_DIRECTORY}/triangle.vert.spv"
    ENGINE_TRIANGLE_FRAGMENT_SHADER="${SHADERS_DIRECTORY}/triangle.frag.spv"
)