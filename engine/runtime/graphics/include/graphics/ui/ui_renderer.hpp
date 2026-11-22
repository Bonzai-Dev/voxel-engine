#pragma once
#include <cstdint>
#include <memory>
#include <core/shared_pointer.hpp>

namespace Engine::Application {
  class GraphicsWindow;
}

namespace Engine::Renderer {
  class RendererContext;
}

namespace Engine::Scene {
  class SceneRenderer;
}

namespace Engine::Application {
  class UIRenderer {
    public:
      explicit UIRenderer(const SharedPtr<Renderer::RendererContext> &context);

      ~UIRenderer();

      void render(
        const SharedPtr<GraphicsWindow> &window,
        uint32_t width,
        uint32_t height
      );

      void createViewport(const SharedPtr<GraphicsWindow> &window);

    private:
      std::unique_ptr<Scene::SceneRenderer> sceneRenderer;
  };
}
