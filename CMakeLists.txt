cmake_minimum_required(VERSION 4.0)
project(voxel-engine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(tools/cmake/shaders.cmake)

set(${CMAKE_PROJECT_NAME}_APPLICATION_DIRECTORY "${CMAKE_BINARY_DIR}/application/$<CONFIG>")
set(${CMAKE_PROJECT_NAME}_RESOURCE_DIRECTORY "${${CMAKE_PROJECT_NAME}_APPLICATION_DIRECTORY}/resources")
set(${CMAKE_PROJECT_NAME}_SHADERS_DIRECTORY "${${CMAKE_PROJECT_NAME}_RESOURCE_DIRECTORY}/shaders")

add_subdirectory(engine)
add_subdirectory(application)
