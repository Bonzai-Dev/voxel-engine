#pragma once
#include <NRI.h>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <engine/runtime/core/intrusive_ptr.hpp>

namespace Engine::Application {
  class GraphicsWindow;
}

namespace Engine::Renderer {
  class RendererContext;
}

namespace Engine::Scene {
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
      void createPipeline(Viewport &viewport, RHI::Format format);
      void destroySwapChainResources(Viewport &viewport) noexcept;
      void destroyResources() noexcept;

      SharedPtr<Renderer::RendererContext> context;
      std::unordered_map<const Application::GraphicsWindow*, std::unique_ptr<Viewport>> windowSwapChains;
      RHI::Queue *graphicsQueue = nullptr;
      RHI::Fence *frameFence = nullptr;
      RHI::CommandAllocator *commandAllocator = nullptr;
      RHI::CommandBuffer *commandBuffer = nullptr;
      std::uint64_t frameFenceValue = 0;
  };
}
