#pragma once
#include <core/application/layer.hpp>

namespace Application {
  class AppLayer: public Core::Application::Layer {
    public:
      explicit AppLayer(const Core::Application::Application &application);

      // void render() override;
  };
}
