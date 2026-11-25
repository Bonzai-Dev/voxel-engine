#pragma once
#include <NRI.h>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <core/shared_pointer.hpp>

namespace Engine::Application {
  class GraphicsWindow;
}

namespace Engine::Renderer {
  class RendererContext;
}

namespace Engine::Scene {
  // TODO: Base class for other kinds of renderer
  class SceneRenderer {
    public:
      explicit SceneRenderer(const SharedPtr<Renderer::RendererContext> &rendererContext);

      ~SceneRenderer();

    private:
      SharedPtr<Renderer::RendererContext> context;
  };
}
