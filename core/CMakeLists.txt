cmake_minimum_required(VERSION 4.0)
project(engine-core)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

file(
    GLOB_RECURSE ENGINE_CORE_SOURCES CONFIGURE_DEPENDS
    "core/*.h" "core/*.c" "core/*.hpp" "core/*.cpp" "core/*.h"
)

add_executable(${CMAKE_PROJECT_NAME} ${ENGINE_CORE_SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(thirdparty)

# macros
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