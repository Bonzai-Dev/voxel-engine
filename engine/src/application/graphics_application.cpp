#include <memory>

#include <SDL3/SDL_video.h>

#include <engine/events/input_events.hpp>
#include <engine/events/window_events.hpp>
#include <engine/application/platform_window.hpp>
#include <engine/renderer/renderer_context.hpp>
#include <engine/memory/intrusive_ptr.hpp>
#include <engine/application/graphics_window.hpp>
#include <engine/application/graphics_application.hpp>
#include <engine/ui/ui_renderer.hpp>

using namespace Engine::Events;

namespace {
  using namespace Engine;
  using namespace Engine::UI;
}

namespace Engine::Application {
  GraphicsApplication::GraphicsApplication(const SharedPtr<Renderer::RendererContext> &rendererContext):
  rendererContext(rendererContext), platformApplication(nullptr) {
    EventDispatcher::listen<WindowShown>([&](const WindowShown &event) { onWindowShow(event); });
    EventDispatcher::listen<WindowHidden>([&](const WindowHidden &event) { onWindowHide(event); });
    EventDispatcher::listen<WindowResized>([&](const WindowResized &event) { onWindowResize(event); });

    EventDispatcher::listen<WindowMouseEnter>([&](const WindowMouseEnter &event) { onWindowMouseEnter(event); });
    EventDispatcher::listen<WindowMouseLeave>([&](const WindowMouseLeave &event) { onWindowMouseLeave(event); });
    EventDispatcher::listen<WindowMouseMotion>([&](const WindowMouseMotion &event) { onWindowMouseMotion(event); });

    EventDispatcher::listen<WindowFocusGained>([&](const WindowFocusGained &event) { onWindowFocusGained(event); });
    EventDispatcher::listen<WindowFocusLost>([&](const WindowFocusLost &event) { onWindowFocusLost(event); });
    EventDispatcher::listen<WindowMinimized>([&](const WindowMinimized &event) { onWindowMinimized(event); });
    EventDispatcher::listen<WindowMaximized>([&](const WindowMaximized &event) { onWindowMaximized(event); });
    EventDispatcher::listen<WindowRestored>([&](const WindowRestored &event) { onWindowRestored(event); });
    EventDispatcher::listen<WindowClosed>([&](const WindowClosed &event) { onWindowClose(event); });
    EventDispatcher::listen<WindowExposed>([&](const WindowExposed &event) { onWindowExposed(event); });

    EventDispatcher::listen<KeyPressedEvent>([&](const KeyPressedEvent &event) { onKeyPressed(event); });
    EventDispatcher::listen<KeyReleasedEvent>([&](const KeyReleasedEvent &event) { onKeyReleased(event); });

    platformApplication = SharedPtr<Application::PlatformApplication>::create();
  }

  GraphicsApplication::~GraphicsApplication() {
  }

  SharedPtr<GraphicsWindow> GraphicsApplication::addWindow(
    const SharedPtr<GraphicsWindow> &graphicalWindow,
    bool showImmediately
  ) const {
    const SharedPtr<Application::PlatformWindow> newWindow = createWindow(graphicalWindow);

    windows.push_back(graphicalWindow);

    if (showImmediately)
      graphicalWindow->show();

    return graphicalWindow;
  }

  SharedPtr<Application::PlatformWindow> GraphicsApplication::createWindow(const SharedPtr<GraphicsWindow> &graphicalWindow) const {
    const WindowOptions options = {
      .title = graphicalWindow->getTitle(),
      .width = graphicalWindow->getWidth(),
      .height = graphicalWindow->getHeight(),
      .focusWhenFirstShown = true,
      .fullScreen = graphicalWindow->isFullscreen(),
      .resizable = graphicalWindow->isResizable(),
    };

    SharedPtr<Application::PlatformWindow> newWindow = SharedPtr<Application::PlatformWindow>::create(options);
    graphicalWindow->setNativeWindow(newWindow);

    if (!uiRenderer) {
      uiRenderer = std::make_unique<UIRenderer>(rendererContext);
    }

    uiRenderer->createViewport(graphicalWindow);

    return newWindow;
  }

  void GraphicsApplication::pollInputs() const {
    platformApplication->pollInputs();
  }

  void GraphicsApplication::render() const {
    for (const SharedPtr<GraphicsWindow> &graphicsWindow: windows) {
      const SharedPtr<Application::PlatformWindow> nativeWindow = graphicsWindow->getNativeWindow();
      auto *window = static_cast<SDL_Window*>(nativeWindow->getHandle());

      int width = 0;
      int height = 0;
      if (!SDL_GetWindowSizeInPixels(window, &width, &height)) {
        throw std::runtime_error(std::string("Unable to query drawable size: ") + SDL_GetError());
      }

      uiRenderer->render(
        graphicsWindow,
        width > 0 ? static_cast<std::uint32_t>(width) : 0,
        height > 0 ? static_cast<std::uint32_t>(height) : 0
      );
    }
  }

  void GraphicsApplication::onWindowShow(const WindowShown &event) {

  }

  void GraphicsApplication::onWindowHide(const WindowHidden &event) {

  }

  void GraphicsApplication::onWindowResize(const WindowResized &event) {

  }

  void GraphicsApplication::onWindowFocusGained(const WindowFocusGained &event) {

  }

  void GraphicsApplication::onWindowFocusLost(const WindowFocusLost &event) {

  }

  void GraphicsApplication::onWindowMinimized(const WindowMinimized &event) {

  }

  void GraphicsApplication::onWindowMaximized(const WindowMaximized &event) {

  }

  void GraphicsApplication::onWindowRestored(const WindowRestored &event) {

  }

  void GraphicsApplication::onWindowClose(const WindowClosed &event) {

  }

  void GraphicsApplication::onWindowExposed(const WindowExposed &event) {

  }

  void GraphicsApplication::onWindowMouseMotion(const WindowMouseMotion &event) {

  }

  void GraphicsApplication::onWindowMouseEnter(const WindowMouseEnter &event) {

  }

  void GraphicsApplication::onWindowMouseLeave(const WindowMouseLeave &event) {

  }

  void GraphicsApplication::onKeyPressed(const KeyPressedEvent &event) {

  }

  void GraphicsApplication::onKeyReleased(const KeyReleasedEvent &event) {

  }
}
