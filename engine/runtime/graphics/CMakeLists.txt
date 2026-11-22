add_engine_module(graphics)
target_link_libraries(engine-graphics
    PUBLIC engine-application engine-events
    PRIVATE engine-renderer engine-scene SDL3::SDL3
)
