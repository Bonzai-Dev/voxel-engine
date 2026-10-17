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

#ifndef ENGINE_TRIANGLE_VERTEX_SHADER
#error "The triangle vertex shader path must be configured by CMake"
#endif

#ifndef ENGINE_TRIANGLE_FRAGMENT_SHADER
#error "The triangle fragment shader path must be configured by CMake"
#endif

using namespace Core::Application;

inline uint8_t GetQueuedFrameNum() {
  return 3;
}

inline uint8_t GetOptimalSwapChainTextureNum() {
  return GetQueuedFrameNum() + 1;
}

namespace {
  constexpr nri::VKBindingOffsets VK_BINDING_OFFSETS = {0, 128, 32, 64};

  void checkNri(nri::Result result, const char *operation) {
    if (result != nri::Result::SUCCESS) {
      throw std::runtime_error(
        std::string(operation) + " failed with NRI result " +
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
    checkNri(nri::nriEnumerateAdapters(nullptr, adapterCount), "Enumerate NRI adapters");

    std::vector<nri::AdapterDesc> adapters(adapterCount);
    checkNri(nri::nriEnumerateAdapters(adapters.data(), adapterCount), "Enumerate NRI adapters");

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
    checkNri(nri::nriCreateDevice(deviceDesc, renderingDevice), "Create NRI device");

    checkNri(
      nri::nriGetInterface(*renderingDevice, NRI_INTERFACE(nri::CoreInterface), &coreInterface),
      "Load NRI core interface"
    );
    checkNri(
      nri::nriGetInterface(*renderingDevice, NRI_INTERFACE(nri::SwapChainInterface), &swapChainInterface),
      "Load NRI swap-chain interface"
    );

    checkNri(
      coreInterface.GetQueue(*renderingDevice, nri::QueueType::GRAPHICS, 0, presentQueue),
      "Get NRI graphics queue"
    );

    // Fences
    checkNri(coreInterface.CreateFence(*renderingDevice, 0, frameFence), "Create NRI fence");

    {
      // Swap chain
      nri::SwapChainDesc swapChainDesc = {};
      swapChainDesc.window = getNriWindow(window);
      swapChainDesc.queue = presentQueue;
      swapChainDesc.format = nri::SwapChainFormat::BT709_G22_8BIT;
      swapChainDesc.flags = (window.options.vsync ? nri::SwapChainBits::VSYNC : nri::SwapChainBits::NONE) |
        nri::SwapChainBits::NONE;

      int width, height;
      SDL_GetWindowSizeInPixels(static_cast<SDL_Window*>(window.getHandle()), &width, &height);
      swapChainDesc.width = width;
      swapChainDesc.height = height;
      swapChainDesc.textureNum = GetOptimalSwapChainTextureNum();
      swapChainDesc.queuedFrameNum = GetQueuedFrameNum();
      swapChainDesc.scaling = nri::Scaling::ONE_TO_ONE;
      checkNri(swapChainInterface.CreateSwapChain(*renderingDevice, swapChainDesc, swapChain),
               "Create NRI swap chain");

      uint32_t swapChainTextureNum;
      nri::Texture *const*swapChainTextures = swapChainInterface.
        GetSwapChainTextures(*swapChain, swapChainTextureNum);

      nri::Format swapChainFormat = coreInterface.GetTextureDesc(*swapChainTextures[0]).format;

      for (uint32_t i = 0; i < swapChainTextureNum; i++) {
        nri::TextureViewDesc textureViewDesc = {
          swapChainTextures[i], nri::TextureView::COLOR_ATTACHMENT, swapChainFormat
        };

        nri::Descriptor *colorAttachment = nullptr;
        checkNri(coreInterface.CreateTextureView(textureViewDesc, colorAttachment), "Create color attachment");

        nri::Fence *acquireSemaphore = nullptr;
        checkNri(coreInterface.CreateFence(*renderingDevice, nri::SWAPCHAIN_SEMAPHORE, acquireSemaphore),
                 "Create acquire semaphore");

        nri::Fence *releaseSemaphore = nullptr;
        checkNri(coreInterface.CreateFence(*renderingDevice, nri::SWAPCHAIN_SEMAPHORE, releaseSemaphore),
                 "Create release semaphore");

        SwapChainTexture &swapChainTexture = this->swapChainTextures.emplace_back();

        swapChainTexture = {};
        swapChainTexture.acquireSemaphore = acquireSemaphore;
        swapChainTexture.releaseSemaphore = releaseSemaphore;
        swapChainTexture.texture = swapChainTextures[i];
        swapChainTexture.colorAttachment = colorAttachment;
        swapChainTexture.attachmentFormat = swapChainFormat;
      }
    }

    // Queued frames
    frameResources.resize(GetQueuedFrameNum());
    for (FrameResources &queuedFrame: frameResources) {
      checkNri(coreInterface.CreateCommandAllocator(*presentQueue, queuedFrame.commandAllocator),
               "Create command allocator");
      checkNri(coreInterface.CreateCommandBuffer(*queuedFrame.commandAllocator, queuedFrame.commandBuffer),
               "Create command buffer");
    }
  }

  Renderer::~Renderer() {
  }

  void Renderer::recreateWindowSurface(const Application::Window &window) {
     // Wait for idle
    coreInterface.DeviceWaitIdle(renderingDevice);

    // Destroy old swapchain
    for (SwapChainTexture& swapChainTexture: swapChainTextures) {
      coreInterface.DestroyFence(swapChainTexture.acquireSemaphore);
      coreInterface.DestroyFence(swapChainTexture.releaseSemaphore);
      coreInterface.DestroyDescriptor(swapChainTexture.colorAttachment);
    }

    swapChainInterface.DestroySwapChain(swapChain);

    // Create new swapchain
    nri::SwapChainDesc swapChainDesc = {};
    swapChainDesc.window = getNriWindow(window);
    swapChainDesc.queue = presentQueue;
    swapChainDesc.format = nri::SwapChainFormat::BT709_G22_8BIT;
    swapChainDesc.flags = (window.options.vsync ? nri::SwapChainBits::VSYNC : nri::SwapChainBits::NONE) | nri::SwapChainBits::NONE;

    int width, height;
    SDL_GetWindowSizeInPixels(static_cast<SDL_Window*>(window.getHandle()), &width, &height);
// LOG_CORE_INFO("Recreating swap chain with width: {}, height: {}", width, height);
    swapChainDesc.width = (uint16_t)width;
    swapChainDesc.height = (uint16_t)height;
    swapChainDesc.textureNum = GetOptimalSwapChainTextureNum();
    swapChainDesc.queuedFrameNum = GetQueuedFrameNum();
    swapChainDesc.scaling = nri::Scaling::ONE_TO_ONE;

    checkNri(swapChainInterface.CreateSwapChain(*renderingDevice, swapChainDesc, swapChain), "Create NRI swap chain");

    uint32_t swapChainTextureNum;
    nri::Texture* const* swapChainTextures = swapChainInterface.GetSwapChainTextures(*swapChain, swapChainTextureNum);

    nri::Format swapChainFormat = this->swapChainTextures[0].attachmentFormat;

    this->swapChainTextures.clear();
    for (uint32_t i = 0; i < swapChainTextureNum; i++) {
        nri::TextureViewDesc textureViewDesc = {swapChainTextures[i], nri::TextureView::COLOR_ATTACHMENT, swapChainFormat};

        nri::Descriptor* colorAttachment = nullptr;
        checkNri(coreInterface.CreateTextureView(textureViewDesc, colorAttachment), "Create color attachment");

        nri::Fence* acquireSemaphore = nullptr;
        checkNri(coreInterface.CreateFence(*renderingDevice, nri::SWAPCHAIN_SEMAPHORE, acquireSemaphore), "Create acquire semaphore");

        nri::Fence* releaseSemaphore = nullptr;
        checkNri(coreInterface.CreateFence(*renderingDevice, nri::SWAPCHAIN_SEMAPHORE, releaseSemaphore), "Create release semaphore");

        SwapChainTexture& swapChainTexture = this->swapChainTextures.emplace_back();

        swapChainTexture = {};
        swapChainTexture.acquireSemaphore = acquireSemaphore;
        swapChainTexture.releaseSemaphore = releaseSemaphore;
        swapChainTexture.texture = swapChainTextures[i];
        swapChainTexture.colorAttachment = colorAttachment;
        swapChainTexture.attachmentFormat = swapChainFormat;
    }
  }

  void Renderer::destroySwapChains() {
  }

  void Renderer::renderFrame() {
    {
      if (swapChainResize) {
        swapChainResize = false;
        recreateWindowSurface(window);
      }
    }

    uint32_t currentFrameIndex = framesDrawn % GetQueuedFrameNum();
    const FrameResources &currentFrameResource = frameResources[currentFrameIndex];

    coreInterface.Wait(*frameFence, framesDrawn >= GetQueuedFrameNum() ? (framesDrawn + 1) - GetQueuedFrameNum() : 0);

    coreInterface.ResetCommandAllocator(*currentFrameResource.commandAllocator);

    uint32_t acquireSemaphoreIndex = currentFrameIndex % (uint32_t)swapChainTextures.size();
    nri::Fence *swapChainAcquireSemaphore = swapChainTextures[acquireSemaphoreIndex].acquireSemaphore;

    uint32_t currentSwapChainTextureIndex = 0;
    const nri::Result acquireResult =
        swapChainInterface.AcquireNextTexture(
            *swapChain,
            *swapChainAcquireSemaphore,
            currentSwapChainTextureIndex
        );
    if (acquireResult == nri::Result::OUT_OF_DATE) {
      swapChainResize = true;
      return;
    }
    checkNri(acquireResult, "Acquire next swap-chain texture");
    const SwapChainTexture &swapChainTexture = swapChainTextures[currentSwapChainTextureIndex];

    nri::CommandBuffer &commandBuffer = *currentFrameResource.commandBuffer;
    coreInterface.BeginCommandBuffer(commandBuffer, nullptr);
    {
      nri::TextureBarrierDesc textureBarriers = {};
      textureBarriers.texture = swapChainTexture.texture;
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
      colorAttachmentDesc.descriptor = swapChainTexture.colorAttachment;

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

    // Submit
    {
      nri::FenceSubmitDesc textureAcquiredFence = {};
      textureAcquiredFence.fence = swapChainAcquireSemaphore;
      textureAcquiredFence.stages = nri::StageBits::COLOR_ATTACHMENT;

      nri::FenceSubmitDesc renderingFinishedFence = {};
      renderingFinishedFence.fence = swapChainTexture.releaseSemaphore;

      nri::QueueSubmitDesc queueSubmitDesc = {};
      queueSubmitDesc.waitFences = &textureAcquiredFence;
      queueSubmitDesc.waitFenceNum = 1;
      queueSubmitDesc.commandBuffers = &currentFrameResource.commandBuffer;
      queueSubmitDesc.commandBufferNum = 1;
      queueSubmitDesc.signalFences = &renderingFinishedFence;
      queueSubmitDesc.signalFenceNum = 1;

      coreInterface.QueueSubmit(*presentQueue, queueSubmitDesc);
    }

    // Present
    if (swapChainInterface.QueuePresent(*swapChain, *swapChainTexture.releaseSemaphore) == nri::Result::OUT_OF_DATE) {
      swapChainResize = true;
    }
    { // Signaling after "Present" improves D3D11 performance a bit
      nri::FenceSubmitDesc signalFence = {};
      signalFence.value = framesDrawn + 1;
      signalFence.fence = frameFence;

      nri::QueueSubmitDesc queueSubmitDesc = {};
      queueSubmitDesc.signalFences = &signalFence;
      queueSubmitDesc.signalFenceNum = 1;

      coreInterface.QueueSubmit(*presentQueue, queueSubmitDesc);
    }

    ++framesDrawn;
  }
}
