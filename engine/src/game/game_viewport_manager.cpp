#include <fstream>
#include <SDL3/SDL.h>
#include <engine/runtime/core/intrusive_ptr.hpp>
#include <engine/runtime/graphics/graphics_window.hpp>
#include <engine/runtime/renderer/renderer_context.hpp>
#include <engine/runtime/game/game_viewport_manager.hpp>
#include <engine/runtime/scene/scene_renderer.hpp>

namespace {
  using namespace Engine;

  std::vector<std::uint8_t> readShader(const char *path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
      throw std::runtime_error(std::string("Unable to open triangle shader: ") + path);
    }

    const std::streamsize size = file.tellg();
    if (size <= 0) {
      throw std::runtime_error(std::string("Triangle shader is empty: ") + path);
    }

    std::vector<std::uint8_t> bytecode(static_cast<std::size_t>(size));
    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(bytecode.data()), size)) {
      throw std::runtime_error(std::string("Unable to read triangle shader: ") + path);
    }
    return bytecode;
  }

  RHI::Window getNriWindow(const Application::PlatformWindow *window) {
    auto *sdlWindow = static_cast<SDL_Window*>(window->getHandle());
    if (sdlWindow == nullptr) {
      throw std::runtime_error("Cannot create an NRI swap chain for a null window");
    }

    const SDL_PropertiesID properties = SDL_GetWindowProperties(sdlWindow);
    const char *driver = SDL_GetCurrentVideoDriver();
    if (properties == 0 || driver == nullptr) {
      throw std::runtime_error("Unable to query the SDL window's native handle");
    }

    RHI::Window nriWindow = {};
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
  struct SwapChainImage {
    RHI::Texture *texture = nullptr;
    RHI::Descriptor *colorAttachment = nullptr;
    RHI::Fence *acquireSemaphore = nullptr;
    RHI::Fence *releaseSemaphore = nullptr;
    bool hasBeenPresented = false;
  };

  struct GameViewportManager::SwapChain {
    RHI::SwapChain *swapChain = nullptr;
    RHI::PipelineLayout *pipelineLayout = nullptr;
    RHI::Pipeline *pipeline = nullptr;
    std::vector<SwapChainImage> images;
    uint32_t width = 0;
    uint32_t height = 0;
    uint64_t frameIndex = 0;
  };

  GameViewportManager::GameViewportManager(const SharedPtr<Renderer::RendererContext> &rendererContext):
  graphicsQueue(nullptr),
  frameFence(nullptr),
  rendererContext(rendererContext) {
    rendererContext->GetQueue(rendererContext->getDevice(), RHI::QueueType::GRAPHICS, 0, graphicsQueue);
    (rendererContext->CreateFence(rendererContext->getDevice(), {}, frameFence));
    (rendererContext->CreateCommandAllocator(*graphicsQueue, commandAllocator));
    (rendererContext->CreateCommandBuffer(*commandAllocator, commandBuffer));
  }

  GameViewportManager::~GameViewportManager() {

  }

  void GameViewportManager::render(const SharedPtr<Application::GraphicsWindow> &window) const {
    if (!swapChains.contains(&window))
      return;

    SwapChain &swapChain = swapChains.at(&window);
    uint32_t frameIndex = frameCount % swapChain.images.size();
    RHI::Fence *acquireSemaphore = swapChain.images[frameIndex].acquireSemaphore;

    if (frameCount > 0)
      rendererContext->Wait(*frameFence, frameCount);

    RHI::Result acquireResult = rendererContext->AcquireNextTexture(
      *swapChain.swapChain, *acquireSemaphore, frameIndex
    );

    if (acquireResult == RHI::Result::OUT_OF_DATE) {
      // createSwapChain(viewport, width, height);
      return;
    }
    if (acquireResult == RHI::Result::NOT_READY)
      return;

    rendererContext->ResetCommandAllocator(*commandAllocator);
    rendererContext->BeginCommandBuffer(*commandBuffer, nullptr);

    SwapChainImage &image = swapChain.images[frameIndex];

    RHI::TextureBarrierDesc toColorAttachment = {};
    toColorAttachment.texture = image.texture;
    toColorAttachment.before = {
      RHI::AccessBits::NONE,
      image.hasBeenPresented ? RHI::Layout::PRESENT : RHI::Layout::UNDEFINED,
      RHI::StageBits::NONE
    };
    toColorAttachment.after = {
      RHI::AccessBits::COLOR_ATTACHMENT,
      RHI::Layout::COLOR_ATTACHMENT,
      RHI::StageBits::COLOR_ATTACHMENT
    };
    RHI::BarrierDesc barrier = {};
    barrier.textures = &toColorAttachment;
    barrier.textureNum = 1;
    rendererContext->CmdBarrier(*commandBuffer, barrier);

    RHI::AttachmentDesc colorAttachment = {};
    colorAttachment.descriptor = image.colorAttachment;
    colorAttachment.clearValue.color.f = {0.02f, 0.04f, 0.1f, 1.0f};
    colorAttachment.loadOp = RHI::LoadOp::CLEAR;
    colorAttachment.storeOp = RHI::StoreOp::STORE;







    RHI::RenderingDesc rendering = {};
    rendering.colors = &colorAttachment;
    rendering.colorNum = 1;
    rendererContext->CmdBeginRendering(*commandBuffer, rendering);

    const RHI::Viewport rhiViewport = {
      0.0f, 0.0f, static_cast<float>(window->getWidth()), static_cast<float>(window->getHeight()), 0.0f, 1.0f
    };
    const RHI::Rect scissor = {
      0, 0, static_cast<RHI::Dim_t>(window->getWidth()), static_cast<RHI::Dim_t>(window->getHeight())
    };
    rendererContext->CmdSetViewports(*commandBuffer, &rhiViewport, 1);
    rendererContext->CmdSetScissors(*commandBuffer, &scissor, 1);
    rendererContext->CmdSetPipelineLayout(*commandBuffer, RHI::BindPoint::GRAPHICS, *swapChain.pipelineLayout);
    rendererContext->CmdSetPipeline(*commandBuffer, *swapChain.pipeline);
    rendererContext->CmdDraw(*commandBuffer, {3, 1, 0, 0});
    rendererContext->CmdEndRendering(*commandBuffer);

    RHI::TextureBarrierDesc toPresent = {};
    toPresent.texture = image.texture;
    toPresent.before = {
      RHI::AccessBits::COLOR_ATTACHMENT,
      RHI::Layout::COLOR_ATTACHMENT,
      RHI::StageBits::COLOR_ATTACHMENT
    };
    toPresent.after = {
      RHI::AccessBits::NONE,
      RHI::Layout::PRESENT,
      RHI::StageBits::NONE
    };
    barrier.textures = &toPresent;
    rendererContext->CmdBarrier(*commandBuffer, barrier);
    (rendererContext->EndCommandBuffer(*commandBuffer));

    const uint64_t nextFrameCount = frameCount + 1;
    RHI::FenceSubmitDesc waitAcquire = {};
    waitAcquire.fence = acquireSemaphore;
    waitAcquire.stages = RHI::StageBits::COLOR_ATTACHMENT;

    RHI::FenceSubmitDesc signalFences[] = {
      {image.releaseSemaphore, 0, RHI::StageBits::ALL},
      {frameFence, nextFrameCount, RHI::StageBits::ALL}
    };
    RHI::CommandBuffer *commandBuffers[] = {commandBuffer};

    RHI::QueueSubmitDesc submit = {};
    submit.waitFences = &waitAcquire;
    submit.waitFenceNum = 1;
    submit.commandBuffers = commandBuffers;
    submit.commandBufferNum = 1;
    submit.signalFences = signalFences;
    submit.signalFenceNum = 2;
    (rendererContext->QueueSubmit(*graphicsQueue, submit));







    image.hasBeenPresented = true;
    frameCount = nextFrameCount;
    ++swapChain.frameIndex;

    const RHI::Result presentResult = rendererContext->QueuePresent(*swapChain.swapChain, *image.releaseSemaphore, 0);
    if (presentResult == RHI::Result::OUT_OF_DATE) {
      // createSwapChain(viewport, width, height);
      return;
    }
  }

  void GameViewportManager::createViewport(const SharedPtr<Application::GraphicsWindow> &window) const {
    if (swapChains.contains(&window))
      return;

    RHI::SwapChainDesc swapChainDesc = {};
    swapChainDesc.width = window->getWidth();
    swapChainDesc.height = window->getHeight();
    swapChainDesc.format = window->getFormat();
    swapChainDesc.flags = RHI::SwapChainBits::VSYNC;
    swapChainDesc.window = getNriWindow(window->getNativeWindow());
    swapChainDesc.queue = graphicsQueue;
    swapChainDesc.textureNum = 3;

    SwapChain &swapChain = swapChains[&window];
    rendererContext->CreateSwapChain(rendererContext->getDevice(), swapChainDesc, swapChain.swapChain);

    uint32_t textureCount = 0;
    RHI::Texture *const *textures = rendererContext->GetSwapChainTextures(*swapChain.swapChain, textureCount);

    swapChain.images.resize(textureCount);

    const RHI::Format format = rendererContext->GetTextureDesc(*textures[0]).format;
    for (uint32_t i = 0; i < textureCount; ++i) {
      SwapChainImage &image = swapChain.images[i];
      image.texture = textures[i];

      const RHI::TextureViewDesc viewDesc = {
        image.texture, RHI::TextureView::COLOR_ATTACHMENT, format
      };

      rendererContext->CreateTextureView(viewDesc, image.colorAttachment);

      const RHI::FenceDesc semaphoreDesc = {RHI::SWAPCHAIN_SEMAPHORE};
      (rendererContext->CreateFence(rendererContext->getDevice(), semaphoreDesc, image.acquireSemaphore));
      (rendererContext->CreateFence(rendererContext->getDevice(), semaphoreDesc, image.releaseSemaphore));
    }

    /// Creating pipeline TEMPORARY
    RHI::PipelineLayoutDesc layoutDesc = {};
    layoutDesc.shaderStages = RHI::StageBits::VERTEX_SHADER | RHI::StageBits::FRAGMENT_SHADER;
    (rendererContext->CreatePipelineLayout(rendererContext->getDevice(), layoutDesc, swapChain.pipelineLayout));

    const std::vector<std::uint8_t> vertexBytecode = readShader(ENGINE_TRIANGLE_VERTEX_SHADER);
    const std::vector<std::uint8_t> fragmentBytecode = readShader(ENGINE_TRIANGLE_FRAGMENT_SHADER);
    const RHI::ShaderDesc shaders[] = {
      {RHI::StageBits::VERTEX_SHADER, vertexBytecode.data(), vertexBytecode.size(), "main"},
      {RHI::StageBits::FRAGMENT_SHADER, fragmentBytecode.data(), fragmentBytecode.size(), "main"}
    };

    RHI::GraphicsPipelineDesc pipelineDesc = {};
    pipelineDesc.pipelineLayout = swapChain.pipelineLayout;
    pipelineDesc.inputAssembly.topology = RHI::Topology::TRIANGLE_LIST;
    pipelineDesc.rasterization.fillMode = RHI::FillMode::SOLID;
    pipelineDesc.rasterization.cullMode = RHI::CullMode::NONE;

    RHI::ColorAttachmentDesc colorDesc = {};
    colorDesc.format = format;
    colorDesc.colorWriteMask = RHI::ColorWriteBits::RGBA;
    pipelineDesc.outputMerger.colors = &colorDesc;
    pipelineDesc.outputMerger.colorNum = 1;
    pipelineDesc.shaders = shaders;
    pipelineDesc.shaderNum = 2;

    (rendererContext->CreateGraphicsPipeline(rendererContext->getDevice(), pipelineDesc, swapChain.pipeline));
  }
}
