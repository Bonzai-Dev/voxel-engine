#pragma once
#include <core/rhi/rhi.hpp>
#include <core/window.hpp>

namespace Core::Renderer {
  class Renderer {
    public:
      Renderer();

      virtual ~Renderer();

      void createSwapChain(const Window *window, uint32_t width, uint32_t height);

    private:
      RHI::SwapChain *swapChain = nullptr;
      RHI::Queue *graphicsQueue = nullptr;
      RHI::Device *renderingDevice = nullptr;
  };
}
