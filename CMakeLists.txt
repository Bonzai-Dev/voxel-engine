cmake_minimum_required(VERSION 4.0)
project(voxel-engine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Creating executable
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c" "src/*.hpp" "src/*.cpp")
add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(thirdparty)

# Compiling shaders
#file(GLOB_RECURSE PROJECT_SHADERS CONFIGURE_DEPENDS "src/*.slang")
#foreach(SHADER ${PROJECT_SHADERS})
#    get_filename_component(SHADER_NAME ${SHADER} NAME)
#    set(SHADERS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/shaders)
#    add_slang_shader_target(
#        ${SHADER_NAME}
#        ${SHADER}
#    )
#endforeach()


