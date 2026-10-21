#pragma once
#include <unordered_map>
#include <utility>
#include <NRI.h>
#include <queue>
#include <Extensions/NRISwapChain.h>
#include <Extensions/NRIDeviceCreation.h>
#include "Extensions/NRIStreamer.h"

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
      void recreateWindowSurface(const Application::Window &window);

      nri::Fence* frameFence;

      struct QueuedFrame {
        nri::CommandAllocator* commandAllocator;
        nri::CommandBuffer* commandBuffer;
      };

      struct SwapChainImage {
        nri::Fence* releaseSemaphore;
        nri::Fence* acquireSemaphore;
        nri::Texture* texture;
        nri::Descriptor* colorAttachment;
        nri::Format attachmentFormat;
      };

      nri::CoreInterface coreInterface = {};
      nri::SwapChainInterface swapChainInterface = {};

      nri::Device* renderingDevice = nullptr;
      nri::SwapChain* swapChain = nullptr;
      nri::Queue* graphicsQueue = nullptr;

      std::vector<QueuedFrame> queuedFrames = {};
      std::vector<SwapChainImage> swapChainImages;

      const Application::Window &window;
      uint32_t frameCount = 0;
  };
}
