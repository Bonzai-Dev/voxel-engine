add_engine_module(game)
target_link_libraries(engine-game
    PUBLIC engine-graphics engine-renderer engine-rhi
    PRIVATE engine-scene SDL3::SDL3 engine-shaders engine-core
)
