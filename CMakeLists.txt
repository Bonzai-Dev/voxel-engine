cmake_minimum_required(VERSION 4.0)
project(voxel-engine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(tools/cmake/shaders.cmake)

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

#add_custom_target(PROJECT_SHADERS)
#file(
#    GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
#    "application/*.c" "application/*.hpp" "application/*.cpp"
#)
#add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES})
#target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(RESOURCE_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/resources")
set(SHADERS_DIRECTORY "${RESOURCE_DIRECTORY}/shaders")

add_subdirectory(core)
add_subdirectory(application)

# Compiling shaders
#file(GLOB_RECURSE SHADER_SOURCE_FILES CONFIGURE_DEPENDS "src/*.slang" "src/*.slangh")
#foreach(SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
#    add_slang_shader_target(${SHADER_SOURCE} ${SHADERS_DIRECTORY} PROJECT_SHADERS)
#endforeach()
#
#add_dependencies(${CMAKE_PROJECT_NAME} PROJECT_SHADERS)

