file(
    GLOB_RECURSE APPLICATION_SOURCES CONFIGURE_DEPENDS
    "./*.h" "./*.c" "./*.hpp" "./*.cpp" "./*.h"
)

add_executable(${CMAKE_PROJECT_NAME} ${APPLICATION_SOURCES})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE voxel-engine::core)