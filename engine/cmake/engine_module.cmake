function(add_engine_module name)
    cmake_parse_arguments(MODULE "" "" "EXCLUDE_SOURCE_DIRS" ${ARGN})
    set(target "engine-${name}")
    set(includeRoot "${CMAKE_CURRENT_SOURCE_DIR}/include")

    file(GLOB_RECURSE headers CONFIGURE_DEPENDS
        "${includeRoot}/*.h"
        "${includeRoot}/*.hpp"
    )

    file(GLOB_RECURSE sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    )
    foreach(directory IN LISTS MODULE_EXCLUDE_SOURCE_DIRS)
        list(FILTER sources EXCLUDE REGEX "^${CMAKE_CURRENT_SOURCE_DIR}/src/${directory}/")
    endforeach()

    add_library(${target} STATIC ${headers} ${sources})
    add_library(voxel-engine::${name} ALIAS ${target})
    target_include_directories(${target} PUBLIC "$<BUILD_INTERFACE:${includeRoot}>")
    target_link_libraries(${target} PUBLIC engine-options)
endfunction()
