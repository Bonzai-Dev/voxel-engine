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
      explicit Renderer(const Application::Window &window);

      ~Renderer();

      void renderFrame();

      void recreateWindowSurface(const Application::Window &window);

      // void createWindowSurface(const Application::Window &window);

      void updateSwapchain() {
        swapChainResize = true;
      }

    private:
      struct SwapChainTexture {
        nri::Fence *acquireSemaphore;
        nri::Fence *releaseSemaphore;
        nri::Texture *texture;
        nri::Descriptor *colorAttachment;
        nri::Format attachmentFormat;
      };

      struct FrameResources {
        nri::CommandAllocator *commandAllocator;
        nri::CommandBuffer *commandBuffer;
      };

      // void destroySwapChain(uint32_t windowId);

      void destroySwapChains();

      const Application::Window &window;

      nri::CoreInterface coreInterface = {};
      nri::SwapChainInterface swapChainInterface = {};

      std::vector<SwapChainTexture> swapChainTextures;
      std::vector<FrameResources> frameResources;
      bool swapChainResize = false;
      int swapChainWidth = 0;
      int swapChainHeight = 0;
      nri::SwapChain *swapChain = nullptr;

      nri::Device *renderingDevice = nullptr;
      nri::Queue *presentQueue = nullptr;

      nri::Fence *frameFence = nullptr;

      uint32_t framesDrawn = 0;
  };
}
