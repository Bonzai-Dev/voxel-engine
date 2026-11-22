add_engine_module(core EXCLUDE_SOURCE_DIRS unix windows)

if(WIN32)
    target_sources(engine-core PRIVATE src/windows/windows_filesystem.cpp)
elseif(UNIX)
    target_sources(engine-core PRIVATE src/unix/unix_filesystem.cpp)
endif()

target_link_libraries(engine-core PUBLIC quill::quill)
