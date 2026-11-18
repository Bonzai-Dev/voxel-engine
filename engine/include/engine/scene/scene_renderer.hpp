#pragma once
#include <engine/memory/intrusive_ptr.hpp>

namespace Engine::Renderer {
  class RendererContext;

  class SceneRenderer {
    public:
      explicit SceneRenderer(const SharedPtr<RendererContext> &rendererContext);

      ~SceneRenderer();

    private:
      SharedPtr<RendererContext> rendererContext = nullptr;
  };
}
