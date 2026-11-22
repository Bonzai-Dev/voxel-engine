add_engine_module(core EXCLUDE_SOURCE_DIRS unix windows)

if(ENGINE_PLATFORM STREQUAL "Windows")
    target_sources(engine-core PRIVATE src/windows/windows_filesystem.cpp)
elseif(ENGINE_PLATFORM STREQUAL "Unix")
    target_sources(engine-core PRIVATE src/unix/unix_filesystem.cpp)
endif()

target_link_libraries(engine-core PUBLIC quill::quill)
