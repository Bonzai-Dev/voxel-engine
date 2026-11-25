include("cmake/engine_module.cmake")

# Macros that will be used by the different modules
add_library(engine-options INTERFACE)
target_compile_features(engine-options INTERFACE cxx_std_20)

if(WIN32)
    target_compile_definitions(engine-options INTERFACE ENGINE_PLATFORM_WINDOWS)
    set(ENGINE_PLATFORM "Windows")
elseif(APPLE)
    target_compile_definitions(engine-options INTERFACE ENGINE_PLATFORM_APPLE)
    set(ENGINE_PLATFORM "Apple")
elseif(UNIX)
    target_compile_definitions(engine-options INTERFACE ENGINE_PLATFORM_LINUX)
    set(ENGINE_PLATFORM "Unix")
endif()

if(MSVC)
    target_compile_definitions(engine-options INTERFACE ENGINE_COMPILER_MSVC)
    set(ENGINE_COMPILER "MSVC")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_definitions(engine-options INTERFACE ENGINE_COMPILER_GCC)
    set(ENGINE_COMPILER "GCC")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$") # TODO: Verify?
    target_compile_definitions(engine-options INTERFACE ENGINE_COMPILER_CLANG)
    set(ENGINE_COMPILER "Clang")
endif()


# Creating engine libraries
add_subdirectory(thirdparty)

add_library(engine-shaders INTERFACE)
file(GLOB_RECURSE shaders CONFIGURE_DEPENDS "runtime/*.slang" "application/*.slang")
foreach(shader IN LISTS shaders)
    add_shader_target(
        "${shader}"
        "${${CMAKE_PROJECT_NAME}_SHADERS_DIRECTORY}"
        engine-shaders
    )
endforeach()


# Linking modules
add_subdirectory(runtime/core)

add_subdirectory(runtime/math)
add_subdirectory(runtime/application)
add_subdirectory(runtime/events)

add_subdirectory(runtime/game)
add_subdirectory(runtime/scene)

add_subdirectory(runtime/graphics)

add_subdirectory(runtime/renderer)
add_subdirectory(runtime/rhi)

add_library(engine INTERFACE)
add_library(voxel-engine::engine ALIAS engine)

target_link_libraries(
    engine INTERFACE
    engine-core
    engine-application
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
