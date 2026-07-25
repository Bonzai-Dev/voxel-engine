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
#if(WIN32)
#    find_path(VULKAN_INCLUDE_DIRECTORY NAMES vulkan/vulkan.h PATHS "$ENV{VULKAN_SDK}/Include")
#
#    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#        find_library(VULKAN_LIBRARY NAMES vulkan-1 PATHS "$ENV{VULKAN_SDK}/Lib" "$ENV{VULKAN_SDK}/Bin")
#    elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
#        find_library(VULKAN_LIBRARY NAMES vulkan-1 PATHS "$ENV{VULKAN_SDK}/Lib32" "$ENV{VULKAN_SDK}/Bin32")
#    endif ()
#else ()
#    find_path(VULKAN_INCLUDE_DIRECTORY NAMES vulkan/vulkan.h PATHS "$ENV{VULKAN_SDK}/include")
#    find_library(VULKAN_LIBRARY NAMES vulkan PATHS "$ENV{VULKAN_SDK}/lib")
#endif ()
#
#if (VULKAN_LIBRARY AND VULKAN_INCLUDE_DIRECTORY AND $ENV{VULKAN_SDK})
#    message(STATUS "Found Vulkan library at ${VULKAN_LIBRARY}")
#    message(STATUS "Found Vulkan include directory at ${VULKAN_INCLUDE_DIRECTORY}")
#    message(STATUS "Found Vulkan SDK at $ENV{VULKAN_SDK}")
#else ()
#    message(STATUS "Vulkan SDK not found. Check if environment variables are set properly.")
#endif ()
#
#add_library(Vulkan::Vulkan UNKNOWN IMPORTED)
#set_target_properties(
#    Vulkan::Vulkan PROPERTIES IMPORTED_LOCATION ${VULKAN_LIBRARY} INTERFACE_INCLUDE_DIRECTORIES ${VULKAN_INCLUDE_DIRECTORY}
#)
#target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Vulkan::Vulkan)

find_package(Vulkan REQUIRED)
message(STATUS "Found Vulkan library: ${Vulkan_LIBRARIES}")
message(STATUS "Found Vulkan include directory: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Found Vulkan SDK: $ENV{VULKAN_SDK}")

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Vulkan::Vulkan)

# ShaderMake
#set(SHADERMAKE_FIND_FXC OFF CACHE BOOL "" FORCE)
#set(SHADERMAKE_FIND_DXC OFF CACHE BOOL "" FORCE)
#set(SHADERMAKE_FIND_DXC_VK OFF CACHE BOOL "" FORCE)
#set(SHADERMAKE_TOOL ON CACHE BOOL "" FORCE)
#set(SHADERMAKE_FIND_SLANG ON CACHE BOOL "" FORCE)
#set(SHADERMAKE_SLANG_PATH "C:/SDKs/slang" CACHE PATH "" FORCE)
#add_subdirectory(ShaderMake EXCLUDE_FROM_ALL)
