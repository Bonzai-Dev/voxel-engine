#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <SDL3/SDL.h>

#include <core/filesystem.hpp>
#include <core/shared_pointer.hpp>

#include <graphics/graphics_window.hpp>
#include <renderer/renderer_context.hpp>
#include <scene/scene_renderer.hpp>
#include <game/game_viewport_manager.hpp>

namespace {
  using namespace Engine;

  nri::Window getNriWindow(const Application::PlatformWindow *window) {
    auto *sdlWindow = static_cast<SDL_Window*>(window->getHandle());
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

namespace Engine::Game {
  struct Frame {
    nri::Texture *texture = nullptr;
    nri::Descriptor *colorAttachment = nullptr;

    nri::Fence *acquireSemaphore = nullptr;
    nri::Fence *releaseSemaphore = nullptr;

    nri::CommandAllocator *commandAllocator = nullptr;
    nri::CommandBuffer *commandBuffer = nullptr;
  };

  struct GameViewportManager::SwapChain {
    nri::SwapChain *swapChain = nullptr;
    nri::PipelineLayout *pipelineLayout = nullptr;
    nri::Pipeline *pipeline = nullptr;
    std::vector<Frame> images;
    uint32_t width = 0;
    uint32_t height = 0;
  };

  GameViewportManager::GameViewportManager(const SharedPtr<Renderer::RendererContext> &rendererContext):
    graphicsQueue(nullptr),
    frameFence(nullptr),
    rendererContext(rendererContext) {
    rendererContext->GetQueue(rendererContext->getDevice(), nri::QueueType::GRAPHICS, 0, graphicsQueue);
    (rendererContext->CreateFence(rendererContext->getDevice(), {}, frameFence));
  }

  GameViewportManager::~GameViewportManager() {
  }

  void GameViewportManager::render(const SharedPtr<Application::GraphicsWindow> &window) const {
    if (!swapChains.contains(&window))
      return;

    SwapChain &swapChain = swapChains.at(&window);
    uint32_t frameIndex = frameCount % swapChain.images.size();
    Frame &currentFrame = swapChain.images[frameIndex];
    nri::Fence *acquireSemaphore = currentFrame.acquireSemaphore;
    nri::CommandBuffer *commandBuffer = currentFrame.commandBuffer;

    {
      if (frameCount > swapChain.images.size())
        rendererContext->Wait(*frameFence, frameCount);

      nri::Result acquireResult = rendererContext->AcquireNextTexture(
        *swapChain.swapChain, *acquireSemaphore, frameIndex
      );

      if (acquireResult == nri::Result::OUT_OF_DATE) {
        // createSwapChain(viewport, width, height);
        return;
      }
      if (acquireResult == nri::Result::NOT_READY)
        return;
    }

    rendererContext->BeginCommandBuffer(*commandBuffer, nullptr);
    {
      nri::TextureBarrierDesc textureBarriers = {};
      textureBarriers.texture = currentFrame.texture;
      textureBarriers.after = {nri::AccessBits::COLOR_ATTACHMENT, nri::Layout::COLOR_ATTACHMENT};

      nri::BarrierDesc barrierDesc = {};
      barrierDesc.textureNum = 1;
      barrierDesc.textures = &textureBarriers;

      rendererContext->CmdBarrier(*commandBuffer, barrierDesc);

      nri::AttachmentDesc colorAttachmentDesc = {};
      colorAttachmentDesc.descriptor = currentFrame.colorAttachment;

      nri::RenderingDesc renderingDesc = {};
      renderingDesc.colorNum = 1;
      renderingDesc.colors = &colorAttachmentDesc;

      rendererContext->CmdBeginRendering(*commandBuffer, renderingDesc);
      {
        rendererContext->CmdSetPipelineLayout(*commandBuffer, nri::BindPoint::GRAPHICS, *swapChain.pipelineLayout);
        rendererContext->CmdSetPipeline(*commandBuffer, *swapChain.pipeline);

        const nri::Viewport viewport = {
          0.0f, 0.0f,
          static_cast<float>(window->getWidth()), static_cast<float>(window->getHeight()),
          0.0f, 1.0f
        };
        rendererContext->CmdSetViewports(*commandBuffer, &viewport, 1);

        nri::Rect scissor = {
          0, 0,
          static_cast<nri::Dim_t>(window->getWidth()), static_cast<nri::Dim_t>(window->getHeight())
        };
        rendererContext->CmdSetScissors(*commandBuffer, &scissor, 1);

        rendererContext->CmdDraw(*commandBuffer, {3, 1, 0, 0});
      }
    }
    rendererContext->CmdEndRendering(*commandBuffer);
    rendererContext->EndCommandBuffer(*commandBuffer);

    const uint64_t nextFrameCount = frameCount + 1;
    {
      // Submit
      const std::array textureAcquiredFences = {
        nri::FenceSubmitDesc{acquireSemaphore, 0, nri::StageBits::COLOR_ATTACHMENT},
      };

      const std::array renderingFinishedFences = {
        nri::FenceSubmitDesc{currentFrame.releaseSemaphore}
      };

      nri::QueueSubmitDesc queueSubmitDesc = {};
      queueSubmitDesc.waitFences = textureAcquiredFences.data();
      queueSubmitDesc.waitFenceNum = textureAcquiredFences.size();
      queueSubmitDesc.commandBuffers = &currentFrame.commandBuffer;
      queueSubmitDesc.commandBufferNum = 1;
      queueSubmitDesc.signalFences = renderingFinishedFences.data();
      queueSubmitDesc.signalFenceNum = renderingFinishedFences.size();

      rendererContext->QueueSubmit(*graphicsQueue, queueSubmitDesc);
    }

    rendererContext->QueuePresent(*swapChain.swapChain, *currentFrame.releaseSemaphore, 0);

    {
      // Signaling after "Present" improves D3D11 performance a bit
      nri::FenceSubmitDesc signalFence = {};
      signalFence.fence = frameFence;
      signalFence.value = nextFrameCount;

      nri::QueueSubmitDesc queueSubmitDesc = {};
      queueSubmitDesc.signalFences = &signalFence;
      queueSubmitDesc.signalFenceNum = 1;

      rendererContext->QueueSubmit(*graphicsQueue, queueSubmitDesc);
    }

    frameCount = nextFrameCount;
  }

  void GameViewportManager::createViewport(const SharedPtr<Application::GraphicsWindow> &window) const {
    if (swapChains.contains(&window))
      return;

    nri::SwapChainDesc swapChainDesc = {};
    swapChainDesc.width = window->getWidth();
    swapChainDesc.height = window->getHeight();
    swapChainDesc.format = window->getFormat();
    swapChainDesc.flags = nri::SwapChainBits::VSYNC;
    swapChainDesc.window = getNriWindow(window->getNativeWindow().get());
    swapChainDesc.queue = graphicsQueue;
    swapChainDesc.textureNum = 3;

    SwapChain &swapChain = swapChains.insert({&window, SwapChain()}).first->second;
    rendererContext->CreateSwapChain(rendererContext->getDevice(), swapChainDesc, swapChain.swapChain);

    uint32_t textureCount = 0;
    nri::Texture *const *textures = rendererContext->GetSwapChainTextures(*swapChain.swapChain, textureCount);

    swapChain.images.resize(textureCount);

    const nri::Format format = rendererContext->GetTextureDesc(*textures[0]).format;
    for (uint32_t i = 0; i < textureCount; ++i) {
      Frame &image = swapChain.images[i];
      image.texture = textures[i];

      const nri::TextureViewDesc viewDesc = {
        image.texture, nri::TextureView::COLOR_ATTACHMENT, format
      };

      rendererContext->CreateTextureView(viewDesc, image.colorAttachment);

      constexpr nri::FenceDesc semaphoreDesc = {nri::SWAPCHAIN_SEMAPHORE};
      (rendererContext->CreateFence(rendererContext->getDevice(), semaphoreDesc, image.acquireSemaphore));
      (rendererContext->CreateFence(rendererContext->getDevice(), semaphoreDesc, image.releaseSemaphore));

      (rendererContext->CreateCommandAllocator(*graphicsQueue, image.commandAllocator));
      (rendererContext->CreateCommandBuffer(*image.commandAllocator, image.commandBuffer));
    }

    /// Creating pipeline TEMPORARY
    nri::PipelineLayoutDesc layoutDesc = {};
    layoutDesc.shaderStages = nri::StageBits::VERTEX_SHADER | nri::StageBits::FRAGMENT_SHADER;
    (rendererContext->CreatePipelineLayout(rendererContext->getDevice(), layoutDesc, swapChain.pipelineLayout));

    const std::vector<std::uint8_t> vertexBytecode = Core::FileSystem::readBytes(
      "resources/shaders/triangle_vertex.spv"
    );
    const std::vector<std::uint8_t> fragmentBytecode = Core::FileSystem::readBytes(
      "resources/shaders/triangle_fragment.spv"
    );
    const nri::ShaderDesc shaders[] = {
      {nri::StageBits::VERTEX_SHADER, vertexBytecode.data(), vertexBytecode.size(), "main"},
      {nri::StageBits::FRAGMENT_SHADER, fragmentBytecode.data(), fragmentBytecode.size(), "main"}
    };

    nri::GraphicsPipelineDesc pipelineDesc = {};
    pipelineDesc.pipelineLayout = swapChain.pipelineLayout;
    pipelineDesc.inputAssembly.topology = nri::Topology::TRIANGLE_LIST;
    pipelineDesc.rasterization.fillMode = nri::FillMode::SOLID;
    pipelineDesc.rasterization.cullMode = nri::CullMode::NONE;

    nri::ColorAttachmentDesc colorDesc = {};
    colorDesc.format = format;
    colorDesc.colorWriteMask = nri::ColorWriteBits::RGBA;
    pipelineDesc.outputMerger.colors = &colorDesc;
    pipelineDesc.outputMerger.colorNum = 1;
    pipelineDesc.shaders = shaders;
    pipelineDesc.shaderNum = 2;

    (rendererContext->CreateGraphicsPipeline(rendererContext->getDevice(), pipelineDesc, swapChain.pipeline));
  }
}
