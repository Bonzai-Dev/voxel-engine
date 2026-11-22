add_engine_module(scene)
target_link_libraries(engine-scene
    PUBLIC engine-core engine-rhi
    PRIVATE engine-graphics engine-renderer SDL3::SDL3 engine-shaders
)
