cmake_minimum_required(VERSION 4.0)
project(voxel-engine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
add_subdirectory(thirdparty)

# Creating executable
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c" "src/*.hpp" "src/*.cpp")
add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Linking
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SDL3::SDL3 glad glm::glm simdjson stb)

# Resource files
add_custom_target(
    PROJECT_ASSETS
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/src/app/res" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/res"
)
add_dependencies(${PROJECT_NAME} PROJECT_ASSETS)