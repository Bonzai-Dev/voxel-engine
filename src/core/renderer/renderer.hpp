#pragma once
#include <core/rhi/rhi.hpp>
#include <core/rhi/extensions/swap_chain.hpp>
#include <core/window.hpp>

namespace Core::Renderer {
  class Renderer {
    public:
      Renderer();

      virtual ~Renderer();

      void createSwapChain(const Window *window, uint32_t width, uint32_t height);

    private:
      RHI::CoreInterface coreInterface = {};
      RHI::SwapChainInterface swapChainInterface = {};

      RHI::SwapChain *swapChain = nullptr;
      RHI::Queue *graphicsQueue = nullptr;
      RHI::Device *renderingDevice = nullptr;

      RHI::PipelineLayout *pipelineLayout = nullptr;
      RHI::PipelineCache *pipelineCache = nullptr;
      RHI::Pipeline *pipeline = nullptr;
      RHI::Pipeline *pipelineMultiview = nullptr;
  };
}
