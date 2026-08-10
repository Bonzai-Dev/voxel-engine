# Slang
set(SLANG_ENABLE_PCH OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_SLANG_RHI OFF CACHE BOOL "" FORCE)
set(SLANG_LIB_TYPE "STATIC" CACHE STRING "" FORCE)
set(SLANG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_GFX OFF CACHE BOOL "" FORCE)

set(SLANG_ENABLE_CUDA OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_OPTIX OFF CACHE BOOL "" FORCE)
add_subdirectory(
    slang
    ${CMAKE_BINARY_DIR}/thirdparty/slang
    EXCLUDE_FROM_ALL
)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE slang)

# SDL
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(SDL EXCLUDE_FROM_ALL)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SDL3::SDL3)

# glm
add_subdirectory(glm EXCLUDE_FROM_ALL)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glm::glm)

#simdjson
add_subdirectory(simdjson EXCLUDE_FROM_ALL)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE simdjson)

# spdlog
add_subdirectory(spdlog EXCLUDE_FROM_ALL)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE spdlog)

# Vulkan
find_package(Vulkan REQUIRED)
message(STATUS "Found Vulkan library: ${Vulkan_LIBRARIES}")
message(STATUS "Found Vulkan include directory: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Found Vulkan SDK: $ENV{VULKAN_SDK}")

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Vulkan::Vulkan)
