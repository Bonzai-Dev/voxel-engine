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

# Compiling shaders
file(
    GLOB_RECURSE ${CMAKE_PROJECT_NAME}_SHADERS CONFIGURE_DEPENDS
    "src/*.slang"
)

foreach(shader IN LISTS ${CMAKE_PROJECT_NAME}_SHADERS)
    add_slang_shader_target(
        "${shader}"
        "${${CMAKE_PROJECT_NAME}_SHADERS_DIRECTORY}"
        engine
    )
endforeach()

target_compile_definitions(
    engine PRIVATE
    ENGINE_TRIANGLE_VERTEX_SHADER="${${CMAKE_PROJECT_NAME}_SHADERS_DIRECTORY}/triangle_vertex.spv"
    ENGINE_TRIANGLE_FRAGMENT_SHADER="${${CMAKE_PROJECT_NAME}_SHADERS_DIRECTORY}/triangle_fragment.spv"
)
