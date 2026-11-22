include("cmake/engine_module.cmake")

# Macros that will be used by the different modules
add_library(engine-options INTERFACE)
target_compile_features(engine-options INTERFACE cxx_std_20)

add_subdirectory(thirdparty)

add_library(engine-shaders INTERFACE)
file(GLOB_RECURSE engine_shaders CONFIGURE_DEPENDS "runtime/renderer/src/*.slang")
foreach(shader IN LISTS engine_shaders)
    add_slang_shader_target(
        "${shader}"
        "${${CMAKE_PROJECT_NAME}_SHADERS_DIRECTORY}"
        engine-shaders
    )
endforeach()
target_compile_definitions(
    engine-shaders INTERFACE
    ENGINE_TRIANGLE_VERTEX_SHADER="${${CMAKE_PROJECT_NAME}_SHADERS_DIRECTORY}/triangle_vertex.spv"
    ENGINE_TRIANGLE_FRAGMENT_SHADER="${${CMAKE_PROJECT_NAME}_SHADERS_DIRECTORY}/triangle_fragment.spv"
)

add_subdirectory(runtime/core)
add_subdirectory(runtime/events)
add_subdirectory(runtime/application)
add_subdirectory(runtime/game)
add_subdirectory(runtime/graphics)
add_subdirectory(runtime/math)
add_subdirectory(runtime/renderer)
add_subdirectory(runtime/rhi)
add_subdirectory(runtime/scene)

add_library(engine INTERFACE)
add_library(voxel-engine::engine ALIAS engine)

target_link_libraries(
    engine INTERFACE
    engine-application
    engine-core
    engine-events
    engine-game
    engine-graphics
    engine-math
    engine-renderer
    engine-rhi
    engine-scene
)

target_compile_definitions(
    engine-options INTERFACE
    $<$<CONFIG:Debug>:ENGINE_DEBUG>
    $<$<CONFIG:Release>:ENGINE_RELEASE>
)

# Defines macros for different platforms
if(WIN32)
    target_compile_definitions(engine-options INTERFACE ENGINE_PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(engine-options INTERFACE ENGINE_PLATFORM_APPLE)
elseif(UNIX)
    target_compile_definitions(engine-options INTERFACE ENGINE_PLATFORM_LINUX)
endif()

if(MSVC)
    target_compile_definitions(engine-options INTERFACE ENGINE_COMPILER_MSVC)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_definitions(engine-options INTERFACE ENGINE_COMPILER_GCC)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$") # TODO: Verify?
    target_compile_definitions(engine-options INTERFACE ENGINE_COMPILER_CLANG)
endif()
