#pragma once
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <NRI.h>
#include <core/shared_pointer.hpp>

namespace Engine::Application {
  class UIRenderer;
  class GraphicsWindow;
}

namespace Engine::Renderer {
  class RendererContext;
}

#include <graphics/viewport_manager.hpp>

namespace Engine::Scene {
  class SceneRenderer;
}

namespace Engine::Game {
  class GameViewportManager: public Application::ViewportManager {
    public:
      GameViewportManager(const SharedPtr<Renderer::RendererContext> &rendererContext);

      ~GameViewportManager() override;

      void createViewport(const SharedPtr<Application::GraphicsWindow> &window) const override;

      void render(const SharedPtr<Application::GraphicsWindow> &window) const override;

    private:
      struct SwapChain;

      std::unique_ptr<Scene::SceneRenderer> sceneRenderer;
      // mutable std::unique_ptr<Application::UIRenderer> uiRenderer;

      mutable std::unordered_map<const SharedPtr<Application::GraphicsWindow>*, SwapChain> swapChains;
      nri::Queue *graphicsQueue;
      nri::Fence *frameFence;
      mutable uint64_t frameCount = 0;

      nri::CommandAllocator *commandAllocator = nullptr;
      nri::CommandBuffer *commandBuffer = nullptr;

      SharedPtr<Renderer::RendererContext> rendererContext;
  };
}
