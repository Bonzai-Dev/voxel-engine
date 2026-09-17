#include <core/logger.hpp>
#include <core/rhi/extensions/swap_chain.hpp>
#include <core/rhi/rhi.hpp>
#include "renderer.hpp"

namespace {
  inline uint8_t getQueuedFrameCount() {
    // return m_Vsync ? 2 : 3;
    return 3;
  }

  inline uint8_t getOptimalSwapChainTextureNum() {
    return getQueuedFrameCount() + 1;
  }
}

namespace Core::Renderer {
  Renderer::Renderer() {
    RHI::DeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.enableGraphicsAPIValidation = true;
    RHI::createDevice(deviceCreateInfo, renderingDevice);


  }

  Renderer::~Renderer() {
    if (renderingDevice) {
      renderingDevice->destroySwapChain(swapChain);

      renderingDevice->deviceWaitIdle();
      RHI::destroyDevice(renderingDevice);
    }
  }

  void Renderer::createSwapChain(const Window *window, uint32_t width, uint32_t height) {
    renderingDevice->getQueue(RHI::QueueType::Graphics, 0, graphicsQueue);

    RHI::SwapChainInfo swapChainInfo = {};

    swapChainInfo.flags |= RHI::SwapChainBits::AllowTearing;
    if (window->options.vsync)
      swapChainInfo.flags |= RHI::SwapChainBits::VSync;
    else
      swapChainInfo.flags |= RHI::SwapChainBits::None;

    swapChainInfo.windowHandle = window->getWindowHandle();
    swapChainInfo.presentQueue = graphicsQueue;
    swapChainInfo.format = RHI::SwapChainFormat::BT709_G22_8BIT;
    swapChainInfo.width = width;
    swapChainInfo.height = height;
    swapChainInfo.textureCount = getOptimalSwapChainTextureNum();
    swapChainInfo.queuedFrameCount = getQueuedFrameCount();

    renderingDevice->createSwapChain(swapChainInfo, swapChain);

    //   uint32_t swapChainTextureNum;
    //   nri::Texture* const* swapChainTextures = NRI.GetSwapChainTextures(*m_SwapChain, swapChainTextureNum);
    //
    //   swapChainFormat = NRI.GetTextureDesc(*swapChainTextures[0]).format;
    //
    //   for (uint32_t i = 0; i < swapChainTextureNum; i++) {
    //       nri::TextureViewDesc textureViewDesc = {swapChainTextures[i], nri::TextureView::COLOR_ATTACHMENT, swapChainFormat};
    //
    //       nri::Descriptor* colorAttachment = nullptr;
    //       NRI_ABORT_ON_FAILURE(NRI.CreateTextureView(textureViewDesc, colorAttachment));
    //
    //       nri::Fence* acquireSemaphore = nullptr;
    //       NRI_ABORT_ON_FAILURE(NRI.CreateFence(*m_Device, nri::SWAPCHAIN_SEMAPHORE, acquireSemaphore));
    //
    //       nri::Fence* releaseSemaphore = nullptr;
    //       NRI_ABORT_ON_FAILURE(NRI.CreateFence(*m_Device, nri::SWAPCHAIN_SEMAPHORE, releaseSemaphore));
    //
    //       SwapChainTexture& swapChainTexture = m_SwapChainTextures.emplace_back();
    //
    //       swapChainTexture = {};
    //       swapChainTexture.acquireSemaphore = acquireSemaphore;
    //       swapChainTexture.releaseSemaphore = releaseSemaphore;
    //       swapChainTexture.texture = swapChainTextures[i];
    //       swapChainTexture.colorAttachment = colorAttachment;
    //       swapChainTexture.attachmentFormat = swapChainFormat;
    //   }
  }
}
