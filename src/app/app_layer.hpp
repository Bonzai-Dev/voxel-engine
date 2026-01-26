#pragma once
#include <core/application/render_layer.hpp>
#include "camera.hpp"
#include "terrain/world.hpp"

class AppLayer : public Core::RenderLayer {
  public:
    explicit AppLayer(const Core::Application &application);

    void render() override;

  private:
    Game::Camera camera;
    Game::World world;
};
