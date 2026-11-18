#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include <SDL3/SDL_video.h>
#include <engine/memory/intrusive_ptr.hpp>
#include <engine/renderer/renderer_context.hpp>
#include <engine/application/platform_window.hpp>
#include <engine/application/graphics_window.hpp>
#include <engine/ui/ui_renderer.hpp>

#ifndef ENGINE_TRIANGLE_VERTEX_SHADER
#error "ENGINE_TRIANGLE_VERTEX_SHADER must name the compiled triangle vertex shader"
#endif

#ifndef ENGINE_TRIANGLE_FRAGMENT_SHADER
#error "ENGINE_TRIANGLE_FRAGMENT_SHADER must name the compiled triangle fragment shader"
#endif

namespace {
  using namespace Engine;

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

  constexpr std::uint8_t swapChainTextureCount = 3;

  void checkResult(RHI::Result result, const char *operation) {
    if (result != RHI::Result::SUCCESS) {
      throw std::runtime_error(
        std::string(operation) + " failed with RHI result " +
        std::to_string(static_cast<int>(result))
      );
    }
  }

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
}

namespace Engine::UI {
  struct UIRenderer::SwapChainImage {
    RHI::Texture *texture = nullptr;
    RHI::Descriptor *colorAttachment = nullptr;
    RHI::Fence *acquireSemaphore = nullptr;
    RHI::Fence *releaseSemaphore = nullptr;
    bool hasBeenPresented = false;
  };

  struct UIRenderer::Viewport {
    SharedPtr<Application::GraphicsWindow> window;
    RHI::SwapChain *swapChain = nullptr;
    RHI::PipelineLayout *pipelineLayout = nullptr;
    RHI::Pipeline *pipeline = nullptr;
    std::vector<SwapChainImage> images;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint64_t frameIndex = 0;
  };

  UIRenderer::UIRenderer(const SharedPtr<Renderer::RendererContext> &rendererContext): context(rendererContext) {
    if (!context) {
      throw std::invalid_argument("UIRenderer requires a renderer context");
    }

    checkResult(
      context->GetQueue(context->getDevice(), RHI::QueueType::GRAPHICS, 0, graphicsQueue),
      "GetQueue"
    );
    if (graphicsQueue == nullptr) {
      throw std::invalid_argument("UIRenderer requires a graphics queue");
    }

    try {
      checkResult(context->CreateFence(context->getDevice(), {}, frameFence), "CreateFence");
      checkResult(context->CreateCommandAllocator(*graphicsQueue, commandAllocator), "CreateCommandAllocator");
      checkResult(context->CreateCommandBuffer(*commandAllocator, commandBuffer), "CreateCommandBuffer");
    }
    catch (...) {
      destroyResources();
      throw;
    }
  }

  UIRenderer::~UIRenderer() {
    destroyResources();
  }

  void UIRenderer::createViewport(const SharedPtr<Application::GraphicsWindow> &window) {
    if (!window) {
      throw std::invalid_argument("Cannot create a renderer viewport for a null window");
    }
    if (windowSwapChains.contains(window.get())) {
      return;
    }

    auto viewport = std::make_unique<Viewport>();
    viewport->window = window;
    windowSwapChains.emplace(window.get(), std::move(viewport));
  }

  bool UIRenderer::createSwapChain(Viewport &viewport, std::uint32_t width, std::uint32_t height) {
    if (width > std::numeric_limits<RHI::Dim_t>::max() ||
      height > std::numeric_limits<RHI::Dim_t>::max()) {
      throw std::out_of_range("UIRenderer extent exceeds the RHI dimension range");
    }

    if (viewport.swapChain != nullptr) {
      if (frameFenceValue != 0) {
        context->Wait(*frameFence, frameFenceValue);
      }
      checkResult(context->DeviceWaitIdle(&context->getDevice()), "DeviceWaitIdle");
      destroySwapChainResources(viewport);
    }

    const SharedPtr<Application::PlatformWindow> nativeWindow = viewport.window->getNativeWindow();
    if (!nativeWindow) {
      throw std::runtime_error("Cannot create an NRI swap chain for a window without a native window");
    }

    RHI::SwapChainDesc swapChainDesc = {};
    swapChainDesc.window = getNriWindow(nativeWindow.get());
    swapChainDesc.queue = graphicsQueue;
    swapChainDesc.format = viewport.window->getFormat();
    swapChainDesc.flags = RHI::SwapChainBits::VSYNC;
    swapChainDesc.width = width;
    swapChainDesc.height = height;
    swapChainDesc.textureNum = swapChainTextureCount;

    const RHI::Result createResult = context->CreateSwapChain(
      context->getDevice(), swapChainDesc, viewport.swapChain
    );
    if (createResult == RHI::Result::NOT_READY) {
      viewport.swapChain = nullptr;
      return false;
    }
    checkResult(createResult, "CreateSwapChain");

    try {
      std::uint32_t textureCount = 0;
      RHI::Texture *const *textures = context->GetSwapChainTextures(*viewport.swapChain, textureCount);
      if (textures == nullptr || textureCount == 0 || textures[0] == nullptr) {
        throw std::runtime_error("Swap chain returned no valid images");
      }

      const RHI::Format format = context->GetTextureDesc(*textures[0]).format;
      viewport.images.resize(textureCount);
      for (std::uint32_t i = 0; i < textureCount; ++i) {
        if (textures[i] == nullptr) {
          throw std::runtime_error("Swap chain returned a null image");
        }

        SwapChainImage &image = viewport.images[i];
        image.texture = textures[i];

        const RHI::TextureViewDesc viewDesc = {
          image.texture, RHI::TextureView::COLOR_ATTACHMENT, format
        };
        checkResult(context->CreateTextureView(viewDesc, image.colorAttachment), "CreateTextureView");

        const RHI::FenceDesc semaphoreDesc = {RHI::SWAPCHAIN_SEMAPHORE};
        checkResult(context->CreateFence(context->getDevice(), semaphoreDesc, image.acquireSemaphore),
                    "CreateFence(acquire semaphore)");
        checkResult(context->CreateFence(context->getDevice(), semaphoreDesc, image.releaseSemaphore),
                    "CreateFence(release semaphore)");
      }

      createPipeline(viewport, format);
      viewport.width = width;
      viewport.height = height;
      viewport.frameIndex = 0;
      return true;
    }
    catch (...) {
      destroySwapChainResources(viewport);
      throw;
    }
  }

  void UIRenderer::createPipeline(Viewport &viewport, RHI::Format format) {
    RHI::PipelineLayoutDesc layoutDesc = {};
    layoutDesc.shaderStages = RHI::StageBits::VERTEX_SHADER | RHI::StageBits::FRAGMENT_SHADER;
    checkResult(context->CreatePipelineLayout(context->getDevice(), layoutDesc, viewport.pipelineLayout),
                "CreatePipelineLayout");

    const std::vector<std::uint8_t> vertexBytecode = readShader(ENGINE_TRIANGLE_VERTEX_SHADER);
    const std::vector<std::uint8_t> fragmentBytecode = readShader(ENGINE_TRIANGLE_FRAGMENT_SHADER);
    const RHI::ShaderDesc shaders[] = {
      {RHI::StageBits::VERTEX_SHADER, vertexBytecode.data(), vertexBytecode.size(), "main"},
      {RHI::StageBits::FRAGMENT_SHADER, fragmentBytecode.data(), fragmentBytecode.size(), "main"}
    };

    RHI::GraphicsPipelineDesc pipelineDesc = {};
    pipelineDesc.pipelineLayout = viewport.pipelineLayout;
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

    checkResult(context->CreateGraphicsPipeline(context->getDevice(), pipelineDesc, viewport.pipeline),
                "CreateGraphicsPipeline");
  }

  void UIRenderer::destroySwapChainResources(Viewport &viewport) noexcept {
    if (!context) {
      return;
    }

    if (viewport.pipeline != nullptr) {
      context->DestroyPipeline(viewport.pipeline);
      viewport.pipeline = nullptr;
    }
    if (viewport.pipelineLayout != nullptr) {
      context->DestroyPipelineLayout(viewport.pipelineLayout);
      viewport.pipelineLayout = nullptr;
    }

    for (SwapChainImage &image: viewport.images) {
      if (image.colorAttachment != nullptr) {
        context->DestroyDescriptor(image.colorAttachment);
        image.colorAttachment = nullptr;
      }
      if (image.acquireSemaphore != nullptr) {
        context->DestroyFence(image.acquireSemaphore);
        image.acquireSemaphore = nullptr;
      }
      if (image.releaseSemaphore != nullptr) {
        context->DestroyFence(image.releaseSemaphore);
        image.releaseSemaphore = nullptr;
      }
    }
    viewport.images.clear();

    if (viewport.swapChain != nullptr) {
      context->DestroySwapChain(viewport.swapChain);
      viewport.swapChain = nullptr;
    }

    viewport.width = 0;
    viewport.height = 0;
    viewport.frameIndex = 0;
  }

  void UIRenderer::destroyResources() noexcept {
    if (context && (frameFenceValue != 0 || !windowSwapChains.empty())) {
      const RHI::Result idleResult = context->DeviceWaitIdle(&context->getDevice());
      if (idleResult != RHI::Result::SUCCESS) {
        ENGINE_LOG_ERROR(Renderer::rendererLogger, "DeviceWaitIdle during UIRenderer teardown failed: {}",
                         static_cast<int>(idleResult));
      }
    }

    for (auto &[window, viewport]: windowSwapChains) {
      (void)window;
      if (viewport) {
        destroySwapChainResources(*viewport);
      }
    }
    windowSwapChains.clear();

    if (context && commandBuffer != nullptr) {
      context->DestroyCommandBuffer(commandBuffer);
      commandBuffer = nullptr;
    }
    if (context && commandAllocator != nullptr) {
      context->DestroyCommandAllocator(commandAllocator);
      commandAllocator = nullptr;
    }
    if (context && frameFence != nullptr) {
      context->DestroyFence(frameFence);
      frameFence = nullptr;
    }
  }

  void UIRenderer::render(
    const SharedPtr<Application::GraphicsWindow> &window,
    std::uint32_t width,
    std::uint32_t height
  ) {
    if (!window) {
      throw std::invalid_argument("Cannot render a null window");
    }
    if (width == 0 || height == 0) {
      return;
    }
    if (width > std::numeric_limits<RHI::Dim_t>::max() ||
      height > std::numeric_limits<RHI::Dim_t>::max()) {
      throw std::out_of_range("UIRenderer extent exceeds the RHI dimension range");
    }
    if (frameFenceValue == std::numeric_limits<std::uint64_t>::max()) {
      throw std::overflow_error("UIRenderer frame fence value overflow");
    }

    const auto viewportIt = windowSwapChains.find(window.get());
    if (viewportIt == windowSwapChains.end()) {
      throw std::logic_error("Cannot render a window without a renderer viewport");
    }
    Viewport &viewport = *viewportIt->second;

    if (viewport.swapChain == nullptr ||
      width != viewport.width ||
      height != viewport.height) {
      if (!createSwapChain(viewport, width, height)) {
        return;
      }
    }

    if (frameFenceValue != 0) {
      context->Wait(*frameFence, frameFenceValue);
    }

    const std::size_t acquireIndex = static_cast<std::size_t>(viewport.frameIndex % viewport.images.size());
    RHI::Fence *acquireSemaphore = viewport.images[acquireIndex].acquireSemaphore;
    std::uint32_t imageIndex = 0;
    const RHI::Result acquireResult = context->AcquireNextTexture(
      *viewport.swapChain, *acquireSemaphore, imageIndex
    );
    if (acquireResult == RHI::Result::OUT_OF_DATE) {
      createSwapChain(viewport, width, height);
      return;
    }
    if (acquireResult == RHI::Result::NOT_READY) {
      return;
    }
    checkResult(acquireResult, "AcquireNextTexture");
    if (imageIndex >= viewport.images.size()) {
      throw std::runtime_error("AcquireNextTexture returned an invalid image index");
    }

    context->ResetCommandAllocator(*commandAllocator);
    checkResult(context->BeginCommandBuffer(*commandBuffer, nullptr), "BeginCommandBuffer");

    SwapChainImage &image = viewport.images[imageIndex];
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
    context->CmdBarrier(*commandBuffer, barrier);

    RHI::AttachmentDesc colorAttachment = {};
    colorAttachment.descriptor = image.colorAttachment;
    colorAttachment.clearValue.color.f = {0.02f, 0.04f, 0.1f, 1.0f};
    colorAttachment.loadOp = RHI::LoadOp::CLEAR;
    colorAttachment.storeOp = RHI::StoreOp::STORE;

    RHI::RenderingDesc rendering = {};
    rendering.colors = &colorAttachment;
    rendering.colorNum = 1;
    context->CmdBeginRendering(*commandBuffer, rendering);

    const RHI::Viewport rhiViewport = {
      0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f
    };
    const RHI::Rect scissor = {
      0, 0, static_cast<RHI::Dim_t>(width), static_cast<RHI::Dim_t>(height)
    };
    context->CmdSetViewports(*commandBuffer, &rhiViewport, 1);
    context->CmdSetScissors(*commandBuffer, &scissor, 1);
    context->CmdSetPipelineLayout(*commandBuffer, RHI::BindPoint::GRAPHICS, *viewport.pipelineLayout);
    context->CmdSetPipeline(*commandBuffer, *viewport.pipeline);
    context->CmdDraw(*commandBuffer, {3, 1, 0, 0});
    context->CmdEndRendering(*commandBuffer);

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
    context->CmdBarrier(*commandBuffer, barrier);
    checkResult(context->EndCommandBuffer(*commandBuffer), "EndCommandBuffer");

    const std::uint64_t nextFenceValue = frameFenceValue + 1;
    RHI::FenceSubmitDesc waitAcquire = {};
    waitAcquire.fence = acquireSemaphore;
    waitAcquire.stages = RHI::StageBits::COLOR_ATTACHMENT;

    RHI::FenceSubmitDesc signalFences[] = {
      {image.releaseSemaphore, 0, RHI::StageBits::ALL},
      {frameFence, nextFenceValue, RHI::StageBits::ALL}
    };
    RHI::CommandBuffer *commandBuffers[] = {commandBuffer};

    RHI::QueueSubmitDesc submit = {};
    submit.waitFences = &waitAcquire;
    submit.waitFenceNum = 1;
    submit.commandBuffers = commandBuffers;
    submit.commandBufferNum = 1;
    submit.signalFences = signalFences;
    submit.signalFenceNum = 2;
    checkResult(context->QueueSubmit(*graphicsQueue, submit), "QueueSubmit");

    image.hasBeenPresented = true;
    frameFenceValue = nextFenceValue;
    ++viewport.frameIndex;

    const RHI::Result presentResult = context->QueuePresent(*viewport.swapChain, *image.releaseSemaphore, 0);
    if (presentResult == RHI::Result::OUT_OF_DATE) {
      createSwapChain(viewport, width, height);
      return;
    }
    checkResult(presentResult, "QueuePresent");
  }
}
