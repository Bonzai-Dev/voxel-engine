#pragma once
#include <core/application.hpp>
#include <core/render_layer.hpp>
#include "game/camera.hpp"
#include "game/terrain/terrain.hpp"

class AppLayer : public Core::RenderLayer {
  public:
    explicit AppLayer(const Core::Application &application);

    void render() override;

  private:
    Game::Terrain terrain;
    Game::Camera camera;
};
