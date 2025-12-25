#pragma once

namespace Core {
  class RenderLayer {
    public:
      RenderLayer() = default;

      virtual ~RenderLayer() = default;

      virtual void Render() {}
  };
}