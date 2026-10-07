file(
    GLOB_RECURSE ENGINE_CORE_SOURCES CONFIGURE_DEPENDS
    "src/*.h" "src/*.c" "src/*.hpp" "src/*.cpp" "src/*.h"
)

# RHI
# Exclude the entire rhi subdirectory
list(FILTER ENGINE_CORE_SOURCES EXCLUDE REGEX ".*/src/rhi/.*")
set(RHI_COMMON_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/rhi.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/rhi.hpp"
)

set(RHI_PLATFORM_SOURCES "")
set(ENGINE_ENABLE_WGPU "ENGINE_ENABLE_WGPU")
set(ENGINE_ENABLE_VULKAN "ENGINE_ENABLE_VULKAN")
set(ENGINE_ENABLE_D3D11 "ENGINE_ENABLE_D3D11")
set(ENGINE_ENABLE_D3D12 "ENGINE_ENABLE_D3D12")

if(WIN32)
    file(
        GLOB_RECURSE RHI_PLATFORM_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/d3d12/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/d3d12/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/d3d12/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/d3d12/*.cpp"
    )
    target_compile_definitions(engine-core PRIVATE ${ENGINE_ENABLE_D3D12})
    target_compile_definitions(engine-core PRIVATE ${ENGINE_ENABLE_D3D11})
    target_compile_definitions(engine-core PRIVATE ${ENGINE_ENABLE_VULKAN})
    message(STATUS "Enabled RHI backend: " D3D12)
    message(STATUS "Enabled RHI backend: " D3D11)
    message(STATUS "Enabled RHI backend: " Vulkan)
elseif(APPLE)
    file(
        GLOB_RECURSE RHI_PLATFORM_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/metal/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/metal/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/metal/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/metal/*.cpp"
    )
    message(STATUS "Enabled RHI backend: " Vulkan)
elseif(UNIX)
    message(STATUS "Enabled RHI backend: " Vulkan)
    file(
        GLOB_RECURSE RHI_PLATFORM_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/vulkan/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/vulkan/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/vulkan/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/vulkan/*.cpp"
    )
else()
    message(FATAL_ERROR "Unsupported platform for RHI backend")
endif()

list(APPEND ENGINE_CORE_SOURCES
    ${RHI_COMMON_SOURCES}
    ${RHI_PLATFORM_SOURCES}
)

add_library(engine-core STATIC ${ENGINE_CORE_SOURCES})
add_library(voxel-engine::core ALIAS engine-core)

set(ENGINE_PUBLIC_INCLUDE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/generated_include")

file(GLOB_RECURSE ENGINE_CORE_HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

foreach(HEADER IN LISTS ENGINE_CORE_HEADERS)
    file(RELATIVE_PATH HEADER_REL "${CMAKE_CURRENT_SOURCE_DIR}/src" "${HEADER}")
    set(DEST "${ENGINE_PUBLIC_INCLUDE_ROOT}/core/${HEADER_REL}")
    get_filename_component(DEST_DIR "${DEST}" DIRECTORY)
    file(MAKE_DIRECTORY "${DEST_DIR}")
    configure_file("${HEADER}" "${DEST}" COPYONLY)
endforeach()

target_include_directories(engine-core
    PUBLIC
    "${ENGINE_PUBLIC_INCLUDE_ROOT}"
)

if(WIN32)
    target_compile_definitions(engine-core PRIVATE ENGINE_ENABLE_D3D12 ENGINE_ENABLE_D3D11 ENGINE_ENABLE_VULKAN)
elseif(APPLE)
    target_compile_definitions(engine-core PRIVATE ENGINE_ENABLE_VULKAN)
elseif(UNIX)
    target_compile_definitions(engine-core PRIVATE ENGINE_ENABLE_VULKAN)
endif()

add_subdirectory(thirdparty)