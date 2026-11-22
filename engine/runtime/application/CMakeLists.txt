add_engine_module(application)
target_link_libraries(engine-application
    PUBLIC engine-core engine-rhi
    PRIVATE engine-events SDL3::SDL3
)
