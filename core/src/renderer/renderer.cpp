#include <../../include/core/renderer/renderer.hpp>

#include <SDL3/SDL_video.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef ENGINE_TRIANGLE_VERTEX_SHADER
#error "The triangle vertex shader path must be configured by CMake"
#endif

#ifndef ENGINE_TRIANGLE_FRAGMENT_SHADER
#error "The triangle fragment shader path must be configured by CMake"
#endif

using namespace Core::Application;

namespace {
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
    auto *sdlWindow = static_cast<SDL_Window*>(window.getWindowHandle());
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
  Renderer::Renderer(const Window &window) {
    try {
      initialize(window);
    }
    catch (...) {
      destroy();
      throw;
    }
  }

  Renderer::~Renderer() {
    destroy();
  }

  void Renderer::initialize(const Window &window) {
    width = static_cast<nri::Dim_t>(window.options.width);
    height = static_cast<nri::Dim_t>(window.options.height);
    if (width == 0 || height == 0 ||
      width != window.options.width || height != window.options.height) {
      throw std::invalid_argument("NRI swap-chain dimensions must fit in a non-zero 16-bit value");
    }

    std::uint32_t adapterCount = 0;
    checkNri(nri::nriEnumerateAdapters(nullptr, adapterCount), "Enumerate NRI adapters");
    if (adapterCount == 0) {
      throw std::runtime_error("NRI did not find any graphics adapters");
    }

    std::vector<nri::AdapterDesc> adapters(adapterCount);
    checkNri(nri::nriEnumerateAdapters(adapters.data(), adapterCount), "Read NRI adapters");

    const auto adapter = std::find_if(adapters.begin(), adapters.end(), [](const auto &item) {
      return (item.supportedGraphicsAPIs & nri::GraphicsAPI::VK) != 0;
    });
    if (adapter == adapters.end()) {
      throw std::runtime_error("No NRI adapter supports Vulkan");
    }

    nri::DeviceCreationDesc deviceDesc = {};
    deviceDesc.graphicsAPI = nri::GraphicsAPI::VK;
    deviceDesc.adapterDesc = &*adapter;
    deviceDesc.vkBindingOffsets = {0, 128, 32, 64};
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
      coreInterface.GetQueue(*renderingDevice, nri::QueueType::GRAPHICS, 0, graphicsQueue),
      "Get NRI graphics queue"
    );

    nri::SwapChainDesc swapChainDesc = {};
    swapChainDesc.window = getNriWindow(window);
    swapChainDesc.queue = graphicsQueue;
    swapChainDesc.width = width;
    swapChainDesc.height = height;
    swapChainDesc.textureNum = 3;
    swapChainDesc.format = nri::SwapChainFormat::BT709_G22_8BIT;
    swapChainDesc.flags = window.options.vsync ? nri::SwapChainBits::VSYNC : nri::SwapChainBits::NONE;
    swapChainDesc.queuedFrameNum = 1;
    checkNri(
      swapChainInterface.CreateSwapChain(*renderingDevice, swapChainDesc, swapChain),
      "Create NRI swap chain"
    );

    std::uint32_t textureCount = 0;
    nri::Texture *const *textures = swapChainInterface.GetSwapChainTextures(*swapChain, textureCount);
    if (textures == nullptr || textureCount == 0) {
      throw std::runtime_error("NRI created a swap chain without any textures");
    }

    swapChainFormat = coreInterface.GetTextureDesc(*textures[0]).format;
    swapChainTextures.resize(textureCount);
    for (std::uint32_t i = 0; i < textureCount; ++i) {
      SwapChainTexture &texture = swapChainTextures[i];
      texture.texture = textures[i];

      const nri::TextureViewDesc viewDesc = {
        texture.texture, nri::TextureView::COLOR_ATTACHMENT, swapChainFormat
      };
      checkNri(coreInterface.CreateTextureView(viewDesc, texture.colorAttachment), "Create color attachment");
      checkNri(
        coreInterface.CreateFence(*renderingDevice, nri::SWAPCHAIN_SEMAPHORE, texture.releaseFence),
        "Create swap-chain release fence"
      );
    }

    checkNri(
      coreInterface.CreateFence(*renderingDevice, nri::SWAPCHAIN_SEMAPHORE, acquireFence),
      "Create swap-chain acquire fence"
    );
    checkNri(coreInterface.CreateFence(*renderingDevice, 0, frameFence), "Create frame fence");
    checkNri(coreInterface.CreateCommandAllocator(*graphicsQueue, commandAllocator), "Create command allocator");
    checkNri(coreInterface.CreateCommandBuffer(*commandAllocator, commandBuffer), "Create command buffer");

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
  }

  void Renderer::render() {
    if (frameIndex != 0) {
      coreInterface.Wait(*frameFence, frameIndex);
    }
    coreInterface.ResetCommandAllocator(*commandAllocator);

    std::uint32_t textureIndex = 0;
    checkNri(
      swapChainInterface.AcquireNextTexture(*swapChain, *acquireFence, textureIndex),
      "Acquire swap-chain texture"
    );
    const SwapChainTexture &texture = swapChainTextures.at(textureIndex);

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

    nri::RenderingDesc rendering = {};
    rendering.colors = &colorAttachment;
    rendering.colorNum = 1;
    coreInterface.CmdBeginRendering(*commandBuffer, rendering);
    coreInterface.CmdSetPipelineLayout(*commandBuffer, nri::BindPoint::GRAPHICS, *pipelineLayout);
    coreInterface.CmdSetPipeline(*commandBuffer, *pipeline);

    const nri::Viewport viewport = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
    const nri::Rect scissor = {0, 0, width, height};
    coreInterface.CmdSetViewports(*commandBuffer, &viewport, 1);
    coreInterface.CmdSetScissors(*commandBuffer, &scissor, 1);
    coreInterface.CmdDraw(*commandBuffer, {3, 1, 0, 0});
    coreInterface.CmdEndRendering(*commandBuffer);

    textureBarrier.before = textureBarrier.after;
    textureBarrier.after = {nri::AccessBits::NONE, nri::Layout::PRESENT, nri::StageBits::NONE};
    coreInterface.CmdBarrier(*commandBuffer, barrier);
    checkNri(coreInterface.EndCommandBuffer(*commandBuffer), "End triangle command buffer");

    const nri::FenceSubmitDesc waitForTexture = {
      acquireFence, 0, nri::StageBits::COLOR_ATTACHMENT
    };
    const nri::FenceSubmitDesc signalFences[] = {
      {texture.releaseFence, 0, nri::StageBits::NONE},
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
    checkNri(coreInterface.QueueSubmit(*graphicsQueue, submitDesc), "Submit triangle frame");
    checkNri(
      swapChainInterface.QueuePresent(*swapChain, *texture.releaseFence),
      "Present triangle frame"
    );
    ++frameIndex;
  }

  void Renderer::destroy() {
    if (graphicsQueue != nullptr) {
      coreInterface.QueueWaitIdle(graphicsQueue);
    }

    if (renderingDevice != nullptr) {
      for (SwapChainTexture &texture : swapChainTextures) {
        if (texture.colorAttachment != nullptr) {
          coreInterface.DestroyDescriptor(texture.colorAttachment);
        }
        if (texture.releaseFence != nullptr) {
          coreInterface.DestroyFence(texture.releaseFence);
        }
      }
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
      if (swapChain != nullptr) {
        swapChainInterface.DestroySwapChain(swapChain);
      }
      nri::nriDestroyDevice(renderingDevice);
    }
  }
}
