#pragma once

#include <core/render_layer.hpp>

class AppLayer : public Core::RenderLayer {
  public:
    AppLayer();

    void Render() override;
};