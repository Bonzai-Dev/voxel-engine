#pragma once
#include <cstdint>
#include <vector>

#include <core/application/window.hpp>
#include <NRI.h>
#include <Extensions/NRIDeviceCreation.h>
#include <Extensions/NRISwapChain.h>

namespace Core::Application {
  class Window;
}

namespace Core::Renderer {
  class Renderer {
    public:
      explicit Renderer(const Application::Window &window);

      ~Renderer();

      void render();

    private:
      struct SwapChainTexture {
        nri::Texture *texture = nullptr;
        nri::Descriptor *colorAttachment = nullptr;
        nri::Fence *releaseFence = nullptr;
      };

      void initialize(const Application::Window &window);
      void destroy();

      nri::CoreInterface coreInterface = {};
      nri::SwapChainInterface swapChainInterface = {};

      nri::SwapChain *swapChain = nullptr;
      nri::Queue *graphicsQueue = nullptr;
      nri::Device *renderingDevice = nullptr;
      nri::CommandAllocator *commandAllocator = nullptr;
      nri::CommandBuffer *commandBuffer = nullptr;
      nri::Fence *frameFence = nullptr;
      nri::Fence *acquireFence = nullptr;
      std::vector<SwapChainTexture> swapChainTextures;
      nri::PipelineLayout *pipelineLayout = nullptr;
      nri::Pipeline *pipeline = nullptr;
      nri::Dim_t width = 0;
      nri::Dim_t height = 0;
      nri::Format swapChainFormat = nri::Format::UNKNOWN;
      uint64_t frameIndex = 0;
  };
}
