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
  Renderer::Renderer() {
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

    checkNri(coreInterface.CreateFence(*renderingDevice, 0, acquireFence), "Create acquire fence");
    checkNri(coreInterface.CreateFence(*renderingDevice, 0, frameFence), "Create frame fence");
    checkNri(coreInterface.CreateCommandAllocator(*presentQueue, commandAllocator), "Create command allocator");
    checkNri(coreInterface.CreateCommandBuffer(*commandAllocator, commandBuffer), "Create command buffer");
  }

  Renderer::~Renderer() {
    coreInterface.DeviceWaitIdle(renderingDevice);

    if (renderingDevice != nullptr) {
      destroySwapChains();

      if (acquireFence != nullptr) {
        coreInterface.DestroyFence(acquireFence);
      }
      if (frameFence != nullptr) {
        coreInterface.DestroyFence(frameFence);
      }
      if (pipeline != nullptr) {
        coreInterface.DestroyPipeline(pipeline);
      }
      if (pipelineLayout != nullptr) {
        coreInterface.DestroyPipelineLayout(pipelineLayout);
      }
      if (commandBuffer != nullptr) {
        coreInterface.DestroyCommandBuffer(commandBuffer);
      }
      if (commandAllocator != nullptr) {
        coreInterface.DestroyCommandAllocator(commandAllocator);
      }
      nri::nriDestroyDevice(renderingDevice);
      renderingDevice = nullptr;
    }
  }

  void Renderer::destroySwapChains() {
    for (auto &[windowId, swapChainTextures]: swapChainTextures) {
      for (SwapChainTexture &texture: swapChainTextures) {
        if (texture.colorAttachment)
          coreInterface.DestroyDescriptor(texture.colorAttachment);
        if (texture.releaseSemaphore)
          coreInterface.DestroyFence(texture.releaseSemaphore);
        if (texture.acquireSemaphore)
          coreInterface.DestroyFence(texture.acquireSemaphore);
      }
    }
    swapChainTextures.clear();

    for (auto &[id, swapChain]: swapChains) {
      if (swapChain)
        swapChainInterface.DestroySwapChain(swapChain);
    }
    swapChains.clear();
  }

  void Renderer::destroySwapChain(uint32_t windowId) {
    for (SwapChainTexture &texture: swapChainTextures.find(windowId)->second) {
      if (texture.colorAttachment)
        coreInterface.DestroyDescriptor(texture.colorAttachment);
      if (texture.releaseSemaphore)
        coreInterface.DestroyFence(texture.releaseSemaphore);
      if (texture.acquireSemaphore)
        coreInterface.DestroyFence(texture.acquireSemaphore);
    }
    swapChainTextures.erase(windowId);

    swapChainInterface.DestroySwapChain(swapChains.find(windowId)->second);
    swapChains.erase(windowId);
  }

  void Renderer::render() {
    std::set<uint32_t> pendingResizes;
    while (!windowResizeQueue.empty()) {
      pendingResizes.insert(windowResizeQueue.front());
      windowResizeQueue.pop();
    }
    for (const uint32_t id: pendingResizes) {
      if (!recreateWindowSurface(*windows[id]))
        windowResizeQueue.push(id); // Zero-sized (minimized) surface, retry next frame
    }

    for (auto &[id, swapChain]: swapChains) {
      coreInterface.Wait(*frameFence, frameIndex);
      coreInterface.ResetCommandAllocator(*commandAllocator);

      uint32_t recycledSemaphoreIndex = frameIndex % (uint32_t)swapChainTextures[id].size();
      nri::Fence* swapChainAcquireSemaphore = swapChainTextures[id][recycledSemaphoreIndex].acquireSemaphore;

      uint32_t currentSwapChainTextureIndex = 0;
      const nri::Result result =
          swapChainInterface.AcquireNextTexture(
              *swapChain,
              *swapChainAcquireSemaphore,
              currentSwapChainTextureIndex);

      if (result == nri::Result::OUT_OF_DATE)
      {
        windowResizeQueue.push(id);
        continue;
      }

      const SwapChainTexture &texture = swapChainTextures[id][currentSwapChainTextureIndex];

      checkNri(coreInterface.BeginCommandBuffer(*commandBuffer, nullptr), "Begin triangle command buffer");
      nri::TextureBarrierDesc textureBarrier = {};
      textureBarrier.texture = texture.texture;
      textureBarrier.after = {nri::AccessBits::COLOR_ATTACHMENT, nri::Layout::COLOR_ATTACHMENT};

      nri::BarrierDesc barrier = {};
      barrier.textures = &textureBarrier;
      barrier.textureNum = 1;
      coreInterface.CmdBarrier(*commandBuffer, barrier);

      nri::AttachmentDesc colorAttachment = {};
      colorAttachment.descriptor = texture.colorAttachment;
      colorAttachment.loadOp = nri::LoadOp::CLEAR;
      colorAttachment.storeOp = nri::StoreOp::STORE;
      colorAttachment.clearValue.color.f = {0.04f, 0.05f, 0.08f, 1.0f};

      nri::RenderingDesc renderingDesc = {};
      renderingDesc.colorNum = 1;
      renderingDesc.colors = &colorAttachment;

      coreInterface.CmdBeginRendering(*commandBuffer, renderingDesc);

      coreInterface.CmdSetPipelineLayout(*commandBuffer, nri::BindPoint::GRAPHICS, *pipelineLayout);
      coreInterface.CmdSetPipeline(*commandBuffer, *pipeline);

      const nri::Viewport viewport = {
        0.0f, 0.0f, static_cast<float>(swapChainDims[id].first), static_cast<float>(swapChainDims[id].second), 0.0f,
        1.0f
      };
      const nri::Rect scissor = {
        0, 0, static_cast<nri::Dim_t>(swapChainDims[id].first), static_cast<nri::Dim_t>(swapChainDims[id].second)
      };
      coreInterface.CmdSetViewports(*commandBuffer, &viewport, 1);
      coreInterface.CmdSetScissors(*commandBuffer, &scissor, 1);
      coreInterface.CmdDraw(*commandBuffer, {3, 1, 0, 0});
      coreInterface.CmdEndRendering(*commandBuffer);

      textureBarrier.before = textureBarrier.after;
      textureBarrier.after = {nri::AccessBits::NONE, nri::Layout::PRESENT, nri::StageBits::NONE};
      coreInterface.CmdBarrier(*commandBuffer, barrier);
      checkNri(coreInterface.EndCommandBuffer(*commandBuffer), "End triangle command buffer");

      const nri::FenceSubmitDesc waitForTexture = {
        swapChainAcquireSemaphore, 0, nri::StageBits::COLOR_ATTACHMENT
      };
      const nri::FenceSubmitDesc signalFences[] = {
        {texture.releaseSemaphore, 0, nri::StageBits::NONE},
        {frameFence, frameIndex + 1, nri::StageBits::NONE},
      };
      nri::CommandBuffer *commandBuffers[] = {commandBuffer};
      const nri::QueueSubmitDesc submitDesc = {
        &waitForTexture,
        1,
        commandBuffers,
        1,
        signalFences,
        2,
        nullptr,
      };
      checkNri(coreInterface.QueueSubmit(*presentQueue, submitDesc), "Submit triangle frame");
      const nri::Result presentResult = swapChainInterface.QueuePresent(*swapChain, *texture.releaseSemaphore);
      if (presentResult == nri::Result::OUT_OF_DATE) {
        windowResizeQueue.push(id);
      }
      ++frameIndex;
    }
  }

  bool Renderer::recreateWindowSurface(const Window &window) {
    coreInterface.DeviceWaitIdle(renderingDevice);
    if (swapChains.contains(window.getId()))
      destroySwapChain(window.getId());
    return createWindowSurface(window);
  }

  bool Renderer::createWindowSurface(const Window &window) {
    // Resize events lag behind the real surface size, so query the live size
    nri::SwapChainDesc swapChainDesc = {};
    swapChainDesc.window = getNriWindow(window);
    swapChainDesc.queue = presentQueue;
    swapChainDesc.width = static_cast<nri::Dim_t>(window.options.width);
    swapChainDesc.height = static_cast<nri::Dim_t>(window.options.height);
    swapChainDesc.textureNum = 3;
    swapChainDesc.format = nri::SwapChainFormat::BT709_G22_8BIT;
    swapChainDesc.flags = window.options.vsync ? nri::SwapChainBits::VSYNC : nri::SwapChainBits::NONE;
    swapChainDesc.queuedFrameNum = 1;
    LOG_CORE_INFO(
        "Creating swap chain: width={}, height={}, window={}x{}",
        swapChainDesc.width,
        swapChainDesc.height,
        window.options.width,
        window.options.height
    );
    if (!windows.contains(window.getId()))
      windows.insert({window.getId(), &window});


    int pixelWidth = 0;
    int pixelHeight = 0;

    SDL_GetWindowSizeInPixels(
        static_cast<SDL_Window*>(window.getHandle()),
        &pixelWidth,
        &pixelHeight
    );

    LOG_CORE_INFO(
        "Resize: SDL={}x{}, options={}x{}, NRI requested={}x{}",
        pixelWidth, pixelHeight,
        window.options.width, window.options.height,
        swapChainDesc.width, swapChainDesc.height
    );

    nri::SwapChain *newSwapChain = nullptr;
    nri::Result createResult = swapChainInterface.CreateSwapChain(*renderingDevice, swapChainDesc, newSwapChain);
    if (createResult == nri::Result::INVALID_ARGUMENT)
      return false; // Surface changed size mid-creation, retry next frame
    checkNri(createResult, "Create NRI swap chain");
    swapChains[window.getId()] = newSwapChain;

    std::uint32_t textureCount = 0;
    nri::Texture *const *textures = swapChainInterface.GetSwapChainTextures(*swapChains[window.getId()], textureCount);
    if (textures == nullptr || textureCount == 0) {
      throw std::runtime_error("NRI created a swap chain without any textures");
    }

    swapChainTextures.insert({window.getId(), std::vector<SwapChainTexture>(textureCount)});
    const nri::TextureDesc &firstTextureDesc = coreInterface.GetTextureDesc(*textures[0]);
    swapChainFormat = firstTextureDesc.format;
    swapChainDims[window.getId()] = {firstTextureDesc.width, firstTextureDesc.height};
    for (size_t i = 0; i < textureCount; ++i) {
      std::vector<SwapChainTexture> &texture = swapChainTextures[window.getId()];
      texture[i].texture = textures[i];
      const nri::TextureViewDesc viewDesc = {
        texture[i].texture, nri::TextureView::COLOR_ATTACHMENT, swapChainFormat
      };

      checkNri(coreInterface.CreateTextureView(viewDesc, texture[i].colorAttachment), "Create color attachment");
      checkNri(coreInterface.CreateFence(*renderingDevice, nri::SWAPCHAIN_SEMAPHORE, texture[i].acquireSemaphore),
               "Create acquire semaphore");
      checkNri(coreInterface.CreateFence(*renderingDevice, nri::SWAPCHAIN_SEMAPHORE, texture[i].releaseSemaphore),
               "Create release semaphore");
    }

    if (pipeline)
      return true;

    // Temporary
    nri::PipelineLayoutDesc layoutDesc = {};
    layoutDesc.shaderStages = nri::StageBits::VERTEX_SHADER | nri::StageBits::FRAGMENT_SHADER;
    checkNri(
      coreInterface.CreatePipelineLayout(*renderingDevice, layoutDesc, pipelineLayout),
      "Create triangle pipeline layout"
    );

    const std::vector<std::uint8_t> vertexShader = loadShader(ENGINE_TRIANGLE_VERTEX_SHADER);
    const std::vector<std::uint8_t> fragmentShader = loadShader(ENGINE_TRIANGLE_FRAGMENT_SHADER);
    const nri::ShaderDesc shaders[] = {
      {nri::StageBits::VERTEX_SHADER, vertexShader.data(), vertexShader.size(), "main"},
      {nri::StageBits::FRAGMENT_SHADER, fragmentShader.data(), fragmentShader.size(), "main"},
    };

    nri::InputAssemblyDesc inputAssembly = {};
    inputAssembly.topology = nri::Topology::TRIANGLE_LIST;

    nri::RasterizationDesc rasterization = {};
    rasterization.fillMode = nri::FillMode::SOLID;
    rasterization.cullMode = nri::CullMode::NONE;

    nri::ColorAttachmentDesc colorDesc = {};
    colorDesc.format = swapChainFormat;
    colorDesc.colorWriteMask = nri::ColorWriteBits::RGBA;

    nri::OutputMergerDesc outputMerger = {};
    outputMerger.colors = &colorDesc;
    outputMerger.colorNum = 1;

    const nri::GraphicsPipelineDesc pipelineDesc = {
      pipelineLayout,
      nullptr,
      inputAssembly,
      rasterization,
      nullptr,
      outputMerger,
      shaders,
      2,
    };
    checkNri(
      coreInterface.CreateGraphicsPipeline(*renderingDevice, pipelineDesc, pipeline),
      "Create triangle graphics pipeline"
    );
    return true;
  }
}
