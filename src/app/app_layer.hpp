#pragma once
#include <core/application.hpp>
#include <core/render_layer.hpp>
#include "game/shaders/default.hpp"
#include "game/camera.hpp"
#include "game/world.hpp"
#include "game/terrain/chunk.hpp"

class AppLayer : public Core::RenderLayer {
  public:
    explicit AppLayer(const Core::Application &application);

    void render() override;

  private:
    Game::World world;
    Game::Camera camera;
    Game::Shader::Default shaderProgram;
    std::vector<Game::Terrain::Chunk> chunks;
};
