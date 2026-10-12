#pragma once
#include <unordered_map>
#include <utility>
#include <NRI.h>
#include <queue>
#include <Extensions/NRISwapChain.h>
#include <Extensions/NRIDeviceCreation.h>

namespace Core::Application {
  class Window;
}

namespace Core::Renderer {
  class Renderer {
    public:
      explicit Renderer();

      ~Renderer();

      void render();

      bool recreateWindowSurface(const Application::Window &window);

      bool createWindowSurface(const Application::Window &window);

    private:
      struct SwapChainTexture {
        nri::Fence *acquireSemaphore;
        nri::Fence *releaseSemaphore;
        nri::Texture *texture;
        nri::Descriptor *colorAttachment;
        nri::Format attachmentFormat;
      };

      void destroySwapChain(uint32_t windowId);

      void destroySwapChains();

      nri::CoreInterface coreInterface = {};
      nri::SwapChainInterface swapChainInterface = {};

      std::unordered_map<uint32_t, nri::SwapChain*> swapChains;
      std::unordered_map<uint32_t, std::vector<SwapChainTexture>> swapChainTextures;
      std::unordered_map<uint32_t, const Application::Window*> windows;
      std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> windowSizes;

      std::queue<uint32_t> windowResizeQueue;
      std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> swapChainDims;

      nri::Device *renderingDevice = nullptr;
      nri::Queue *presentQueue = nullptr;

      nri::CommandAllocator *commandAllocator = nullptr;
      nri::CommandBuffer *commandBuffer = nullptr;

      nri::Fence *frameFence = nullptr;
      nri::Fence *acquireFence = nullptr;

      nri::PipelineLayout *pipelineLayout = nullptr;
      nri::Pipeline *pipeline = nullptr;
      nri::Format swapChainFormat = nri::Format::UNKNOWN;
      uint32_t frameIndex = 0;
  };
}
