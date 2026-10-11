#file(
#    GLOB_RECURSE APPLICATION_SOURCES CONFIGURE_DEPENDS
#    "src/application/*.h" "src/application/*.c" "src/application/*.hpp" "src/application/*.cpp" "src/application/*.h"
#)
#
#file(
#    GLOB_RECURSE EVENTS_SOURCES CONFIGURE_DEPENDS
#    "src/events/*.h" "src/events/*.c" "src/events/*.hpp" "src/events/*.cpp" "src/events/*.h"
#)

# Configuring RHI
#set(ENGINE_ENABLE_NONE ENGINE_RHI_ENABLE_NONE=1)
#set(ENGINE_RHI_ENABLE_VULKAN ENGINE_RHI_ENABLE_VULKAN=1)
#set(ENGINE_ENABLE_D3D11 ENGINE_RHI_ENABLE_D3D11=1)
#set(ENGINE_ENABLE_D3D12 ENGINE_RHI_ENABLE_D3D12=1)
#set(ENGINE_ENABLE_WGPU ENGINE_RHI_ENABLE_WGPU=1)
#
#set(RHI_ENABLED_BACKENDS)
#file(
#    GLOB_RECURSE RHI_COMMON CONFIGURE_DEPENDS
#    "src/rhi/extensions/*.h" "src/rhi/extensions/*.c" "src/rhi/extensions/*.hpp" "src/rhi/extensions/*.cpp" "src/rhi/extensions/*.h"
#    "src/rhi/shared/*.h" "src/rhi/shared/*.c" "src/rhi/shared/*.hpp" "src/rhi/shared/*.cpp" "src/rhi/shared/*.h"
#    "src/rhi/validation/*.h" "src/rhi/validation/*.c" "src/rhi/validation/*.hpp" "src/rhi/validation/*.cpp" "src/rhi/validation/*.h"
#)
#
#file(
#    GLOB_RECURSE RHI_NONE_SOURCES CONFIGURE_DEPENDS
#    "src/rhi/none/*.h" "src/rhi/none/*.c" "src/rhi/none/*.hpp" "src/rhi/none/*.cpp" "src/rhi/none/*.h"
#)
#
#message(STATUS "Enabled RHI backend: None")
#list(APPEND RHI_ENABLED_BACKENDS ${ENGINE_ENABLE_NONE})
#
#if (WIN32)
#    file(
#        GLOB_RECURSE RHI_D3D11_SOURCES CONFIGURE_DEPENDS
#        "src/rhi/D3D11/*.h" "src/rhi/D3D11/*.c" "src/rhi/D3D11/*.hpp" "src/rhi/D3D11/*.cpp" "src/rhi/D3D11/*.h"
#    )
#
#    file(
#        GLOB_RECURSE RHI_D3D12_SOURCES CONFIGURE_DEPENDS
#        "src/rhi/D3D12/*.h" "src/rhi/D3D12/*.c" "src/rhi/D3D12/*.hpp" "src/rhi/D3D12/*.cpp" "src/rhi/D3D12/*.h"
#    )
#
#    file(
#        GLOB_RECURSE RHI_VULKAN_SOURCES CONFIGURE_DEPENDS
#        "src/rhi/vulkan/*.h" "src/rhi/vulkan/*.c" "src/rhi/vulkan/*.hpp" "src/rhi/vulkan/*.cpp" "src/rhi/vulkan/*.h"
#    )
#    message(STATUS "Enabled RHI backend: Vulkan")
#    message(STATUS "Enabled RHI backend: D3D11")
#    message(STATUS "Enabled RHI backend: D3D12")
#
#    list(APPEND RHI_ENABLED_BACKENDS ${ENGINE_ENABLE_D3D11})
#    list(APPEND RHI_ENABLED_BACKENDS ${ENGINE_ENABLE_D3D12})
#    list(APPEND RHI_ENABLED_BACKENDS ${ENGINE_RHI_ENABLE_VULKAN})
#elseif (APPLE)
#    file(
#        GLOB_RECURSE RHI_VULKAN_SOURCES CONFIGURE_DEPENDS
#        "src/rhi/vulkan/*.h" "src/rhi/vulkan/*.c" "src/rhi/vulkan/*.hpp" "src/rhi/vulkan/*.cpp" "src/rhi/vulkan/*.h"
#    )
#    message(STATUS "Enabled RHI backend: Vulkan")
#    list(APPEND RHI_ENABLED_BACKENDS ${ENGINE_RHI_ENABLE_VULKAN})
#elseif (UNIX)
#    file(
#        GLOB_RECURSE RHI_VULKAN_SOURCES CONFIGURE_DEPENDS
#        "src/rhi/vulkan/*.h" "src/rhi/vulkan/*.c" "src/rhi/vulkan/*.hpp" "src/rhi/vulkan/*.cpp" "src/rhi/vulkan/*.h"
#    )
#    message(STATUS "Enabled RHI backend: Vulkan")
#    list(APPEND RHI_ENABLED_BACKENDS ${ENGINE_RHI_ENABLE_VULKAN})
#endif ()
#
#set(RHI_SOURCES ${RHI_D3D11_SOURCES} ${RHI_D3D12_SOURCES} ${RHI_VULKAN_SOURCES} ${RHI_NONE_SOURCES} ${RHI_COMMON})

file(
    GLOB_RECURSE EVENTS_SOURCES CONFIGURE_DEPENDS
    "src/*.h" "src/*.c" "src/*.hpp" "src/*.cpp" "src/*.h"
)

# Creating core library
#add_library(engine-core STATIC ${APPLICATION_SOURCES} ${EVENTS_SOURCES} ${RHI_SOURCES})
add_library(engine-core STATIC ${APPLICATION_SOURCES} ${EVENTS_SOURCES})
add_library(voxel-engine::core ALIAS engine-core)
target_include_directories(engine-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(engine-core PRIVATE ${RHI_ENABLED_BACKENDS})

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
add_dependencies(engine-core engine-triangle-shaders)
target_compile_definitions(
    engine-core PRIVATE
    ENGINE_TRIANGLE_VERTEX_SHADER="${SHADERS_DIRECTORY}/triangle.vert.spv"
    ENGINE_TRIANGLE_FRAGMENT_SHADER="${SHADERS_DIRECTORY}/triangle.frag.spv"
)