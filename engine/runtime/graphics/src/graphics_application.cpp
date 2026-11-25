#include <memory>
#include <utility>
#include <vector>

#include <SDL3/SDL_video.h>

#include <events/input_events.hpp>
#include <events/window_events.hpp>
#include <application/platform_window.hpp>
#include <renderer/renderer_context.hpp>
#include <graphics/ui/ui_renderer.hpp>
#include <scene/scene_renderer.hpp>
#include <core/shared_pointer.hpp>
#include <graphics/graphics_window.hpp>
#include <graphics/graphics_application.hpp>

using namespace Engine::Events;

namespace {
  using namespace Engine;
  // using namespace Engine::Scene;
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

    platformApplication = SharedPtr<PlatformApplication>::create();
  }

  GraphicsApplication::~GraphicsApplication() {
  }

  SharedPtr<GraphicsWindow> GraphicsApplication::addWindow(
    const SharedPtr<GraphicsWindow> &graphicalWindow,
    bool showImmediately
  ) const {
    {
      const WindowOptions options = {
        .title = graphicalWindow->getTitle(),
        .width = graphicalWindow->getInitialWidth(),
        .height = graphicalWindow->getInitialHeight(),
        .focusWhenFirstShown = true,
        .fullScreen = graphicalWindow->isFullscreen(),
        .resizable = graphicalWindow->isResizable(),
      };

      SharedPtr<PlatformWindow> newWindow = SharedPtr<PlatformWindow>::create(options);
      graphicalWindow->setNativeWindow(newWindow);
    }

    windows.push_back(graphicalWindow);

    if (showImmediately)
      graphicalWindow->show();

    return graphicalWindow;
  }

  void GraphicsApplication::pollInputs() const {
    platformApplication->pollInputs();
  }

  // void GraphicsApplication::render() const {
  // for (const SharedPtr<GraphicsWindow> &graphicsWindow: windows) {
  //   const SharedPtr<Application::PlatformWindow> nativeWindow = graphicsWindow->getNativeWindow();
  //   auto *window = static_cast<SDL_Window*>(nativeWindow->getHandle());
  //
  //   int width = 0;
  //   int height = 0;
  //   if (!SDL_GetWindowSizeInPixels(window, &width, &height)) {
  //     throw std::runtime_error(std::string("Unable to query drawable size: ") + SDL_GetError());
  //   }
  //
  //   sceneRenderer->render(
  //     graphicsWindow,
  //     width > 0 ? static_cast<std::uint32_t>(width) : 0,
  //     height > 0 ? static_cast<std::uint32_t>(height) : 0
  //   );
  // }
  // }

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
