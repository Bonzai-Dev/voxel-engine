cmake_minimum_required(VERSION 4.0)
project(voxel-engine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(tools/cmake/shaders.cmake)

set(RESOURCE_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/resources")
set(SHADERS_DIRECTORY "${RESOURCE_DIRECTORY}/shaders")

add_subdirectory(core)
add_subdirectory(application)

# Macros
target_compile_definitions(
    ${CMAKE_PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:ENGINE_DEBUG>
    $<$<CONFIG:Release>:ENGINE_RELEASE>
)

if(WIN32)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE ENGINE_PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE ENGINE_PLATFORM_LINUX)
elseif(APPLE)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE ENGINE_PLATFORM_APPLE)
endif()

if (${MSVC})
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE ENGINE_COMPILER_MSVC)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE ENGINE_COMPILER_GCC)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang") # TODO: validate
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE ENGINE_COMPILER_CLANG)
endif()
