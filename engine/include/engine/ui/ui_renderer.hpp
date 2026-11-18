#pragma once
#include <NRI.h>
#include <memory>
#include <unordered_map>
#include <engine/memory/intrusive_ptr.hpp>

namespace Engine::Renderer {
  class RendererContext;
}

namespace Engine::UI {
  class UIRenderer {
    public:
      explicit UIRenderer(const SharedPtr<Renderer::RendererContext> &context);

      ~UIRenderer();

      void render(
        const SharedPtr<Application::GraphicsWindow> &window,
        uint32_t width,
        uint32_t height
      );

      void createViewport(const SharedPtr<Application::GraphicsWindow> &window);

    private:
      struct SwapChainImage;

      struct Viewport;

      bool createSwapChain(Viewport &viewport, uint32_t width, uint32_t height);
      void createPipeline(Viewport &viewport, RHI::Format format);
      void destroySwapChainResources(Viewport &viewport) noexcept;
      void destroyResources() noexcept;

      SharedPtr<Renderer::RendererContext> context;

      std::unordered_map<const Application::GraphicsWindow*, std::unique_ptr<Viewport>> windowSwapChains;

      RHI::Queue *graphicsQueue = nullptr;

      RHI::Fence *frameFence = nullptr;
      RHI::CommandAllocator *commandAllocator = nullptr;
      RHI::CommandBuffer *commandBuffer = nullptr;

      uint64_t frameFenceValue = 0;
  };
}
