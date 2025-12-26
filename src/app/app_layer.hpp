#pragma once
#include <core/application.hpp>
#include <core/render_layer.hpp>
#include "game/shaders/default.hpp"
#include "game/camera.hpp"

class AppLayer : public Core::RenderLayer {
  public:
    explicit AppLayer(const Core::Application &application);

    void Render() override;

  private:
    Game::Camera m_camera;
    Game::Shader::DefaultShader m_shaderProgram;
    unsigned int m_vao;
    unsigned int m_vbo;
};
