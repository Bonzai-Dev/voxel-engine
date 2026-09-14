#include <core/logger.hpp>
#include <core/rhi/extensions/swap_chain.hpp>
#include <core/rhi/rhi.hpp>
#include "core/rhi/stl/allocator.hpp"
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
    RHI::createDevice(deviceCreateInfo, renderingDevice);

    // Swap chain
    RHI::Format swapChainFormat;
    {

    }
  }

  Renderer::~Renderer() {
    if (renderingDevice) {
      // renderingDevice->deviceWaitIdle();
      RHI::destroy(renderingDevice->allocationCallbacks, renderingDevice);
    }
  }

  void Renderer::createSwapChain(void *windowHandle, uint32_t width, uint32_t height) {
    // renderingDevice->getQueue(RHI::QueueType::Graphics, 0, graphicsQueue);
    //
    // RHI::SwapChainInfo swapChainInfo = {};
    // swapChainInfo.windowHandle = windowHandle;
    // swapChainInfo.presentQueue = graphicsQueue;
    // swapChainInfo.format = RHI::SwapChainFormat::BT709_G22_8BIT;
    // swapChainInfo.flags = RHI::SwapChainBits::VSync;
    // swapChainInfo.width = width;
    // swapChainInfo.height = height;
    // swapChainInfo.textureCount = getOptimalSwapChainTextureNum();
    // swapChainInfo.queuedFrameCount = getQueuedFrameCount();

    // renderingDevice->createSwapChain(swapChainInfo, swapChain);

    // nri::SwapChainDesc swapChainDesc = {};
    //   swapChainDesc.window = GetWindow();
    //   swapChainDesc.queue = m_GraphicsQueue;
    //   swapChainDesc.format = nri::SwapChainFormat::BT709_G22_8BIT;
    //   swapChainDesc.flags = (m_Vsync ? nri::SwapChainBits::VSYNC : nri::SwapChainBits::NONE) | nri::SwapChainBits::ALLOW_TEARING;
    //   swapChainDesc.width = (uint16_t)GetOutputResolution().x;
    //   swapChainDesc.height = (uint16_t)GetOutputResolution().y;
    //   swapChainDesc.textureCount = GetOptimalSwapChainTextureNum();
    //   swapChainDesc.queuedFrameCount = GetQueuedFrameNum();
    //   NRI_ABORT_ON_FAILURE(NRI.CreateSwapChain(*m_Device, swapChainDesc, m_SwapChain));
    //
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
