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

      void render(
        const SharedPtr<Application::GraphicsWindow> &window,
        std::uint32_t width,
        std::uint32_t height
      );

      void createViewport(const SharedPtr<Application::GraphicsWindow> &window);

    private:
      struct SwapChainImage;
      struct Viewport;

      bool createSwapChain(Viewport &viewport, std::uint32_t width, std::uint32_t height);
      void createPipeline(Viewport &viewport, nri::Format format);
      void destroySwapChainResources(Viewport &viewport) noexcept;
      void destroyResources() noexcept;

      SharedPtr<Renderer::RendererContext> context;
      std::unordered_map<const Application::GraphicsWindow*, std::unique_ptr<Viewport>> windowSwapChains;
      nri::Queue *graphicsQueue = nullptr;
      nri::Fence *frameFence = nullptr;
      nri::CommandAllocator *commandAllocator = nullptr;
      nri::CommandBuffer *commandBuffer = nullptr;
      std::uint64_t frameFenceValue = 0;
  };
}
