#include <algorithm>
#include <cstring>
#include <fstream>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL3/SDL_video.h>
#include <core/application/window.hpp>
#include <core/renderer/renderer.hpp>

#include "core/application/logger.hpp"

using namespace Core::Application;
namespace {
  inline uint8_t GetQueuedFrameNum(bool vsync) {
    return vsync ? 2 : 3;
  }

  inline uint8_t GetOptimalSwapChainTextureNum(bool vsync) {
    return GetQueuedFrameNum(vsync) + 1;
  }

  constexpr nri::VKBindingOffsets VK_BINDING_OFFSETS = {0, 128, 32, 64};

  void checkNri(nri::Result result) {
    if (result != nri::Result::SUCCESS) {
      throw std::runtime_error(
        std::to_string(static_cast<int>(result))
      );
    }
  }

  std::vector<std::uint8_t> loadShader(const char *path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
      throw std::runtime_error(std::string("Unable to open shader: ") + path);
    }

    const std::streamsize size = file.tellg();
    if (size <= 0) {
      throw std::runtime_error(std::string("Shader is empty: ") + path);
    }

    std::vector<std::uint8_t> code(static_cast<std::size_t>(size));
    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(code.data()), size)) {
      throw std::runtime_error(std::string("Unable to read shader: ") + path);
    }
    return code;
  }

  nri::Window getNriWindow(const Window &window) {
    auto *sdlWindow = static_cast<SDL_Window*>(window.getHandle());
    if (sdlWindow == nullptr) {
      throw std::runtime_error("Cannot create an NRI swap chain for a null window");
    }

    const SDL_PropertiesID properties = SDL_GetWindowProperties(sdlWindow);
    const char *driver = SDL_GetCurrentVideoDriver();
    if (properties == 0 || driver == nullptr) {
      throw std::runtime_error("Unable to query the SDL window's native handle");
    }

    nri::Window nriWindow = {};
#if defined(_WIN32)
    if (std::strcmp(driver, "windows") != 0) {
      throw std::runtime_error("NRI renderer requires SDL's Windows video driver");
    }
    nriWindow.windows.hwnd = SDL_GetPointerProperty(
      properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr
    );
#elif defined(__linux__)
    if (std::strcmp(driver, "x11") == 0) {
      nriWindow.x11.dpy = SDL_GetPointerProperty(
        properties, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr
      );
      nriWindow.x11.window = static_cast<std::uint64_t>(SDL_GetNumberProperty(
        properties, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0
      ));
    }
    else if (std::strcmp(driver, "wayland") == 0) {
      nriWindow.wayland.display = SDL_GetPointerProperty(
        properties, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr
      );
      nriWindow.wayland.surface = SDL_GetPointerProperty(
        properties, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr
      );
    }
    else {
      throw std::runtime_error("NRI renderer requires SDL's X11 or Wayland video driver");
    }
#else
    (void)driver;
    (void)properties;
    throw std::runtime_error("NRI renderer does not yet support this SDL video driver");
#endif
    return nriWindow;
  }
}

namespace Core::Renderer {
  Renderer::Renderer(const Application::Window &window): window(window) {
    // Adapters
    std::uint32_t adapterCount = 0;
    checkNri(nri::nriEnumerateAdapters(nullptr, adapterCount));

    std::vector<nri::AdapterDesc> adapters(adapterCount);
    checkNri(nri::nriEnumerateAdapters(adapters.data(), adapterCount));

    // The RHI automatically sorts the best adapters first
    const nri::AdapterDesc &adapter = adapters.front();
    nri::DeviceCreationDesc deviceDesc = {};
    deviceDesc.graphicsAPI = nri::GraphicsAPI::VK;
    deviceDesc.adapterDesc = &adapter;
    deviceDesc.vkBindingOffsets = VK_BINDING_OFFSETS;
#ifdef ENGINE_DEBUG
    deviceDesc.enableNRIValidation = true;
    deviceDesc.enableGraphicsAPIValidation = true;
#endif
    checkNri(nri::nriCreateDevice(deviceDesc, renderingDevice));

    checkNri(
      nri::nriGetInterface(*renderingDevice, NRI_INTERFACE(nri::CoreInterface), &coreInterface)
    );
    checkNri(
      nri::nriGetInterface(*renderingDevice, NRI_INTERFACE(nri::SwapChainInterface), &swapChainInterface)
    );

    checkNri(coreInterface.GetQueue(*renderingDevice, nri::QueueType::GRAPHICS, 0, graphicsQueue));

    {
      // Swap chain
      nri::SwapChainDesc swapChainDesc = {};
      swapChainDesc.window = getNriWindow(window);
      swapChainDesc.queue = graphicsQueue;
      swapChainDesc.format = nri::SwapChainFormat::BT709_G22_8BIT;
      swapChainDesc.flags = (window.options.vsync ? nri::SwapChainBits::VSYNC : nri::SwapChainBits::NONE) |
        nri::SwapChainBits::NONE;
      swapChainDesc.width = (uint16_t)window.options.width;
      swapChainDesc.height = (uint16_t)window.options.height;
      swapChainDesc.textureNum = GetOptimalSwapChainTextureNum(window.options.vsync);
      swapChainDesc.queuedFrameNum = GetQueuedFrameNum(window.options.vsync);
      swapChainDesc.scaling = nri::Scaling::ONE_TO_ONE;
      (swapChainInterface.CreateSwapChain(*renderingDevice, swapChainDesc, swapChain));

      uint32_t swapChainTextureNum;
      nri::Texture *const*swapChainTextures = swapChainInterface.GetSwapChainTextures(*swapChain, swapChainTextureNum);

      nri::Format swapChainFormat = coreInterface.GetTextureDesc(*swapChainTextures[0]).format;

      for (uint32_t i = 0; i < swapChainTextureNum; i++) {
        nri::TextureViewDesc textureViewDesc = {
          swapChainTextures[i], nri::TextureView::COLOR_ATTACHMENT, swapChainFormat
        };

        nri::Descriptor *colorAttachment = nullptr;
        (coreInterface.CreateTextureView(textureViewDesc, colorAttachment));

        SwapChainImage &swapChainTexture = swapChainImages.emplace_back();

        (coreInterface.CreateFence(*renderingDevice, nri::SWAPCHAIN_SEMAPHORE, swapChainTexture.releaseSemaphore));

        swapChainTexture.texture = swapChainTextures[i];
        swapChainTexture.colorAttachment = colorAttachment;
        swapChainTexture.attachmentFormat = swapChainFormat;
      }
    }

    queuedFrames.resize(GetQueuedFrameNum(window.options.vsync));
    for (auto &queuedFrame: queuedFrames) {
      coreInterface.CreateFence(*renderingDevice, 0, queuedFrame.frameFence);
      coreInterface.CreateCommandAllocator(*graphicsQueue, queuedFrame.commandAllocator);
      coreInterface.CreateCommandBuffer(*queuedFrame.commandAllocator, queuedFrame.commandBuffer);
      (coreInterface.CreateFence(*renderingDevice, nri::SWAPCHAIN_SEMAPHORE, queuedFrame.acquireSemaphore));
    }
  }

  Renderer::~Renderer() {
  }

  void Renderer::recreateWindowSurface(const Application::Window &window) {
    {
      coreInterface.QueueWaitIdle(graphicsQueue);

      for (auto &swapChainImage: swapChainImages) {
        coreInterface.DestroyDescriptor(swapChainImage.colorAttachment);
        coreInterface.DestroyFence(swapChainImage.releaseSemaphore);
      }
      swapChainImages.clear();

      swapChainInterface.DestroySwapChain(swapChain);

      // Swap chain
      nri::SwapChainDesc swapChainDesc = {};
      swapChainDesc.window = getNriWindow(window);
      swapChainDesc.queue = graphicsQueue;
      swapChainDesc.format = nri::SwapChainFormat::BT709_G22_8BIT;
      swapChainDesc.flags = (window.options.vsync ? nri::SwapChainBits::VSYNC : nri::SwapChainBits::NONE) |
        nri::SwapChainBits::NONE;
      swapChainDesc.width = (uint16_t)window.options.width;
      swapChainDesc.height = (uint16_t)window.options.height;
      swapChainDesc.textureNum = GetOptimalSwapChainTextureNum(window.options.vsync);
      swapChainDesc.queuedFrameNum = GetQueuedFrameNum(window.options.vsync);
      swapChainDesc.scaling = nri::Scaling::ONE_TO_ONE;
      (swapChainInterface.CreateSwapChain(*renderingDevice, swapChainDesc, swapChain));

      uint32_t swapChainTextureNum;
      nri::Texture *const*swapChainTextures = swapChainInterface.GetSwapChainTextures(*swapChain, swapChainTextureNum);

      nri::Format swapChainFormat = coreInterface.GetTextureDesc(*swapChainTextures[0]).format;

      for (uint32_t i = 0; i < swapChainTextureNum; i++) {
        nri::TextureViewDesc textureViewDesc = {
          swapChainTextures[i], nri::TextureView::COLOR_ATTACHMENT, swapChainFormat
        };

        nri::Descriptor *colorAttachment = nullptr;
        (coreInterface.CreateTextureView(textureViewDesc, colorAttachment));

        SwapChainImage &swapChainTexture = swapChainImages.emplace_back();
        swapChainTexture = {};

        (coreInterface.CreateFence(*renderingDevice, nri::SWAPCHAIN_SEMAPHORE, swapChainTexture.releaseSemaphore));

        swapChainTexture.texture = swapChainTextures[i];
        swapChainTexture.colorAttachment = colorAttachment;
        swapChainTexture.attachmentFormat = swapChainFormat;
      }
    }
  }

  void Renderer::render() {
    {
      if (updateSwapChain) {
        updateSwapChain = false;
        recreateWindowSurface(window);
      }
    }

    uint32_t currentFrameIndex = frameCount % GetQueuedFrameNum(window.options.vsync);

    coreInterface.Wait(*queuedFrames[currentFrameIndex].frameFence, queuedFrames[currentFrameIndex].lastFenceValue);
    coreInterface.ResetCommandAllocator(*queuedFrames[currentFrameIndex].commandAllocator);

    uint32_t imageIndex;
    auto res = swapChainInterface.AcquireNextTexture(*swapChain, *queuedFrames[currentFrameIndex].acquireSemaphore, imageIndex);
    if (res == nri::Result::OUT_OF_DATE) {
      updateSwapChain = true;
      return;
    }
    checkNri(res);

    // Record
    nri::CommandBuffer &commandBuffer = *queuedFrames[currentFrameIndex].commandBuffer;
    coreInterface.BeginCommandBuffer(commandBuffer, nullptr);
    {
      nri::TextureBarrierDesc textureBarriers = {};
      textureBarriers.texture = swapChainImages[imageIndex].texture;
      textureBarriers.after = {nri::AccessBits::COPY_SOURCE, nri::Layout::COPY_SOURCE};
      textureBarriers.layerNum = 1;
      textureBarriers.mipNum = 1;

      nri::BarrierDesc barrierDesc = {};
      barrierDesc.textureNum = 1;
      barrierDesc.textures = &textureBarriers;
      coreInterface.CmdBarrier(commandBuffer, barrierDesc);

      nri::TextureDataLayoutDesc dstDataLayoutDesc = {};
      dstDataLayoutDesc.rowPitch = coreInterface.GetDeviceDesc(*renderingDevice).memoryAlignment.uploadBufferTextureRow;

      textureBarriers.before = textureBarriers.after;
      textureBarriers.after = {nri::AccessBits::COLOR_ATTACHMENT, nri::Layout::COLOR_ATTACHMENT};
      coreInterface.CmdBarrier(commandBuffer, barrierDesc);

      nri::AttachmentDesc colorAttachmentDesc = {};
      colorAttachmentDesc.descriptor = swapChainImages[imageIndex].colorAttachment;

      nri::RenderingDesc renderingDesc = {};
      renderingDesc.colorNum = 1;
      renderingDesc.colors = &colorAttachmentDesc;

      coreInterface.CmdBeginRendering(commandBuffer, renderingDesc);
      {
        nri::ClearAttachmentDesc clearDesc = {};
        clearDesc.planes = nri::PlaneBits::COLOR;
        clearDesc.value.color.f = {1.0f, 0.0f, 0.0f, 1.0f};
        coreInterface.CmdClearAttachments(commandBuffer, &clearDesc, 1, nullptr, 0);
      }
      coreInterface.CmdEndRendering(commandBuffer);

      textureBarriers.before = textureBarriers.after;
      textureBarriers.after = {nri::AccessBits::NONE, nri::Layout::PRESENT, nri::StageBits::NONE};

      coreInterface.CmdBarrier(commandBuffer, barrierDesc);
    }
    coreInterface.EndCommandBuffer(commandBuffer);

    { // Submit
      nri::FenceSubmitDesc textureAcquiredFence = {};
      textureAcquiredFence.fence = queuedFrames[currentFrameIndex].acquireSemaphore;
      textureAcquiredFence.stages = nri::StageBits::COLOR_ATTACHMENT;

      nri::FenceSubmitDesc renderingFinishedFence = {};
      renderingFinishedFence.fence = swapChainImages[imageIndex].releaseSemaphore;

      nri::QueueSubmitDesc queueSubmitDesc = {};
      queueSubmitDesc.waitFences = &textureAcquiredFence;
      queueSubmitDesc.waitFenceNum = 1;
      queueSubmitDesc.commandBuffers = &queuedFrames[currentFrameIndex].commandBuffer;
      queueSubmitDesc.commandBufferNum = 1;
      queueSubmitDesc.signalFences = &renderingFinishedFence;
      queueSubmitDesc.signalFenceNum = 1;

      coreInterface.QueueSubmit(*graphicsQueue, queueSubmitDesc);
    }

    // Present
    auto res2 = swapChainInterface.QueuePresent(*swapChain, *swapChainImages[imageIndex].releaseSemaphore);
    if (res2 == nri::Result::OUT_OF_DATE) {
      updateSwapChain = true;
    }

    { // Signaling after "Present" improves D3D11 performance a bit
      nri::FenceSubmitDesc signalFence = {};
      signalFence.fence = queuedFrames[currentFrameIndex].frameFence;
      signalFence.value = queuedFrames[currentFrameIndex].lastFenceValue + 1;

      nri::QueueSubmitDesc queueSubmitDesc = {};
      queueSubmitDesc.signalFences = &signalFence;
      queueSubmitDesc.signalFenceNum = 1;
      queuedFrames[currentFrameIndex].lastFenceValue = signalFence.value;

      coreInterface.QueueSubmit(*graphicsQueue, queueSubmitDesc);
    }

    // frameIndex = (frameIndex + 1) % GetQueuedFrameNum(window.options.vsync);
    ++frameCount;
  }
}
