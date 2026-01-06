#pragma once
#include <core/application.hpp>
#include <core/render_layer.hpp>
#include "game/shaders/default.hpp"
#include "game/camera.hpp"
#include "game/terrain/chunk.hpp"

class AppLayer : public Core::RenderLayer {
  public:
    explicit AppLayer(const Core::Application &application);

    void Render() override;

  private:
    Game::Camera m_camera;
    Game::Shader::Default m_shaderProgram;
    std::vector<Game::Terrain::Chunk> m_chunks;
};
