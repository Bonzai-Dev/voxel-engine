# Slang
set(SLANG_ENABLE_PCH OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_SLANG_RHI ON CACHE BOOL "" FORCE) # TODO: turn this back off
set(SLANG_LIB_TYPE "STATIC" CACHE STRING "" FORCE)
set(SLANG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_GFX OFF CACHE BOOL "" FORCE)
set(SLANG_USE_SYSTEM_VULKAN_HEADERS ON CACHE BOOL "" FORCE)
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
set(SDL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
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
add_subdirectory(vulkan/VulkanMemoryAllocator EXCLUDE_FROM_ALL)
add_subdirectory(vulkan/Vulkan-Headers EXCLUDE_FROM_ALL)

# We are making sure that volk uses the bundled Vulkan headers instead of the system ones
set(VOLK_PULL_IN_VULKAN OFF CACHE BOOL "" FORCE)
add_subdirectory(vulkan/volk EXCLUDE_FROM_ALL)
target_link_libraries(volk PRIVATE Vulkan-Headers)

add_library(VulkanHeaders INTERFACE)
target_link_libraries(VulkanHeaders INTERFACE Vulkan-Headers volk GPUOpen::VulkanMemoryAllocator)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE VulkanHeaders)
