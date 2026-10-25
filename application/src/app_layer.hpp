#pragma once
#include <engine/application/layer.hpp>

namespace Application {
  class AppLayer: public Engine::Application::Layer {
    public:
      explicit AppLayer(const Engine::Application::Application &application);

      // void render() override;
  };
}
