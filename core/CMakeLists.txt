#file(
#    GLOB_RECURSE ENGINE_CORE_SOURCES CONFIGURE_DEPENDS
#    "src/*.h" "src/*.c" "src/*.hpp" "src/*.cpp" "src/*.h"
#)
#
## RHI
## Exclude the entire rhi subdirectory
#list(FILTER ENGINE_CORE_SOURCES EXCLUDE REGEX ".*/src/rhi/.*")
#set(RHI_COMMON_SOURCES
#    "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/rhi.cpp"
#    "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/rhi.hpp"
#)
#
#set(RHI_PLATFORM_SOURCES "")
#set(ENGINE_ENABLE_WGPU ENGINE_ENABLE_WGPU=1)
#set(ENGINE_RHI_ENABLE_VULKAN ENGINE_RHI_ENABLE_VULKAN=1)
#set(ENGINE_ENABLE_D3D11 ENGINE_ENABLE_D3D11=1)
#set(ENGINE_ENABLE_D3D12 ENGINE_ENABLE_D3D12=1)
#
#if(WIN32)
#    file(
#        GLOB_RECURSE RHI_PLATFORM_SOURCES CONFIGURE_DEPENDS
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/d3d12/*.h"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/d3d12/*.hpp"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/d3d12/*.c"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/d3d12/*.cpp"
#    )
#    list(APPEND ENGINE_CORE_COMPILE_DEFINITIONS
#            ${ENGINE_ENABLE_D3D12}
#            ${ENGINE_ENABLE_D3D11}
#            ${ENGINE_RHI_ENABLE_VULKAN}
#    )
#    message(STATUS "Enabled RHI backend: " D3D12)
#    message(STATUS "Enabled RHI backend: " D3D11)
#    message(STATUS "Enabled RHI backend: " Vulkan)
#elseif(APPLE)
#    file(
#        GLOB_RECURSE RHI_PLATFORM_SOURCES CONFIGURE_DEPENDS
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/metal/*.h"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/metal/*.hpp"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/metal/*.c"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/metal/*.cpp"
#    )
#    message(STATUS "Enabled RHI backend: " Vulkan)
#    list(APPEND ENGINE_CORE_COMPILE_DEFINITIONS
#            ${ENGINE_RHI_ENABLE_VULKAN}
#    )
#elseif(UNIX)
#    message(STATUS "Enabled RHI backend: " Vulkan)
#    file(
#        GLOB_RECURSE RHI_PLATFORM_SOURCES CONFIGURE_DEPENDS
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/vulkan/*.h"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/vulkan/*.hpp"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/vulkan/*.c"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/rhi/vulkan/*.cpp"
#    )
#    list(APPEND ENGINE_CORE_COMPILE_DEFINITIONS
#            ${ENGINE_RHI_ENABLE_VULKAN}
#    )
#else()
#    message(FATAL_ERROR "Unsupported platform for RHI backend")
#endif()
#
#list(APPEND ENGINE_CORE_SOURCES
#    ${RHI_COMMON_SOURCES}
#    ${RHI_PLATFORM_SOURCES}
#)
#
#add_library(engine-core STATIC ${ENGINE_CORE_SOURCES})
#add_library(voxel-engine::core ALIAS engine-core)
#target_compile_definitions(engine-core PRIVATE ${ENGINE_CORE_COMPILE_DEFINITIONS})
#
#set(ENGINE_PUBLIC_INCLUDE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/generated_include")
#
#file(GLOB_RECURSE ENGINE_CORE_HEADERS CONFIGURE_DEPENDS
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
#)
#
#foreach(HEADER IN LISTS ENGINE_CORE_HEADERS)
#    file(RELATIVE_PATH HEADER_REL "${CMAKE_CURRENT_SOURCE_DIR}/src" "${HEADER}")
#    set(DEST "${ENGINE_PUBLIC_INCLUDE_ROOT}/core/${HEADER_REL}")
#    get_filename_component(DEST_DIR "${DEST}" DIRECTORY)
#    file(MAKE_DIRECTORY "${DEST_DIR}")
#    configure_file("${HEADER}" "${DEST}" COPYONLY)
#endforeach()
#
#target_include_directories(engine-core
#    PUBLIC
#    "${ENGINE_PUBLIC_INCLUDE_ROOT}"
#)











file(
    GLOB_RECURSE APPLICATION_SOURCES CONFIGURE_DEPENDS
    "src/application/*.h" "src/application/*.c" "src/application/*.hpp" "src/application/*.cpp" "src/application/*.h"
)

file(
    GLOB_RECURSE EVENTS_SOURCES CONFIGURE_DEPENDS
    "src/events/*.h" "src/events/*.c" "src/events/*.hpp" "src/events/*.cpp" "src/events/*.h"
)

# Configuring RHI
set(ENGINE_ENABLE_NONE ENGINE_RHI_ENABLE_NONE=1)
set(ENGINE_RHI_ENABLE_VULKAN ENGINE_RHI_ENABLE_VULKAN=1)
set(ENGINE_ENABLE_D3D11 ENGINE_RHI_ENABLE_D3D11=1)
set(ENGINE_ENABLE_D3D12 ENGINE_RHI_ENABLE_D3D12=1)
set(ENGINE_ENABLE_WGPU ENGINE_RHI_ENABLE_WGPU=1)

set(RHI_ENABLED_BACKENDS)
file(
    GLOB_RECURSE RHI_COMMON CONFIGURE_DEPENDS
    "src/rhi/extensions/*.h" "src/rhi/extensions/*.c" "src/rhi/extensions/*.hpp" "src/rhi/extensions/*.cpp" "src/rhi/extensions/*.h"
    "src/rhi/shared/*.h" "src/rhi/shared/*.c" "src/rhi/shared/*.hpp" "src/rhi/shared/*.cpp" "src/rhi/shared/*.h"
    "src/rhi/validation/*.h" "src/rhi/validation/*.c" "src/rhi/validation/*.hpp" "src/rhi/validation/*.cpp" "src/rhi/validation/*.h"
)

file(
    GLOB_RECURSE RHI_NONE_SOURCES CONFIGURE_DEPENDS
    "src/rhi/none/*.h" "src/rhi/none/*.c" "src/rhi/none/*.hpp" "src/rhi/none/*.cpp" "src/rhi/none/*.h"
)

message(STATUS "Enabled RHI backend: None")
list(APPEND RHI_ENABLED_BACKENDS ${ENGINE_ENABLE_NONE})

if (WIN32)
    file(
        GLOB_RECURSE RHI_D3D11_SOURCES CONFIGURE_DEPENDS
        "src/rhi/D3D11/*.h" "src/rhi/D3D11/*.c" "src/rhi/D3D11/*.hpp" "src/rhi/D3D11/*.cpp" "src/rhi/D3D11/*.h"
    )

    file(
        GLOB_RECURSE RHI_D3D12_SOURCES CONFIGURE_DEPENDS
        "src/rhi/D3D12/*.h" "src/rhi/D3D12/*.c" "src/rhi/D3D12/*.hpp" "src/rhi/D3D12/*.cpp" "src/rhi/D3D12/*.h"
    )

    file(
        GLOB_RECURSE RHI_VULKAN_SOURCES CONFIGURE_DEPENDS
        "src/rhi/vulkan/*.h" "src/rhi/vulkan/*.c" "src/rhi/vulkan/*.hpp" "src/rhi/vulkan/*.cpp" "src/rhi/vulkan/*.h"
    )
    message(STATUS "Enabled RHI backend: Vulkan")
    message(STATUS "Enabled RHI backend: D3D11")
    message(STATUS "Enabled RHI backend: D3D12")

    list(APPEND RHI_ENABLED_BACKENDS ${ENGINE_ENABLE_D3D11})
    list(APPEND RHI_ENABLED_BACKENDS ${ENGINE_ENABLE_D3D12})
    list(APPEND RHI_ENABLED_BACKENDS ${ENGINE_RHI_ENABLE_VULKAN})
elseif (APPLE)
    file(
        GLOB_RECURSE RHI_VULKAN_SOURCES CONFIGURE_DEPENDS
        "src/rhi/vulkan/*.h" "src/rhi/vulkan/*.c" "src/rhi/vulkan/*.hpp" "src/rhi/vulkan/*.cpp" "src/rhi/vulkan/*.h"
    )
    message(STATUS "Enabled RHI backend: Vulkan")
    list(APPEND RHI_ENABLED_BACKENDS ${ENGINE_RHI_ENABLE_VULKAN})
elseif (UNIX)
    file(
        GLOB_RECURSE RHI_VULKAN_SOURCES CONFIGURE_DEPENDS
        "src/rhi/vulkan/*.h" "src/rhi/vulkan/*.c" "src/rhi/vulkan/*.hpp" "src/rhi/vulkan/*.cpp" "src/rhi/vulkan/*.h"
    )
    message(STATUS "Enabled RHI backend: Vulkan")
    list(APPEND RHI_ENABLED_BACKENDS ${ENGINE_RHI_ENABLE_VULKAN})
endif ()

set(RHI_SOURCES ${RHI_D3D11_SOURCES} ${RHI_D3D12_SOURCES} ${RHI_VULKAN_SOURCES} ${RHI_NONE_SOURCES} ${RHI_COMMON})

# Creating core library
add_library(engine-core STATIC ${APPLICATION_SOURCES} ${EVENTS_SOURCES} ${RHI_SOURCES})
add_library(voxel-engine::core ALIAS engine-core)
target_include_directories(engine-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(engine-core PRIVATE ${RHI_ENABLED_BACKENDS})

add_subdirectory(thirdparty)