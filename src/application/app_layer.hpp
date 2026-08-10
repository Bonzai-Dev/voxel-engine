#pragma once
#include <core/layer.hpp>

class AppLayer : public Core::Layer {
  public:
    explicit AppLayer(const Core::Application &application);

    // void render() override;
};
