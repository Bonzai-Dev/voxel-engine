#pragma once
#include <core/application/render_layer.hpp>
#include <core/renderer/camera/camera.hpp>
#include "./game/terrain/terrain.hpp"

class AppLayer : public Core::RenderLayer {
  public:
    explicit AppLayer(const Core::Application &application);

    void render() override;

  private:
    Game::Terrain terrain;
    Renderer::Camera camera;
};
