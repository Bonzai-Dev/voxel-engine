#include <engine/events/input_events.hpp>
#include <engine/events/window_events.hpp>
#include <engine/application/platform_window.hpp>
#include <engine/graphics/graphical_application.hpp>

using namespace Engine::Events;

namespace Engine::Graphics {
  GraphicalApplication::GraphicalApplication() {
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

    platformApplication = SharedPtr<Application::PlatformApplication>().create();
  }

  SharedPtr<GraphicalWindow> GraphicalApplication::addWindow(SharedPtr<GraphicalWindow> graphicalWindow, bool showImmediately) {
    const SharedPtr<Application::PlatformWindow> newWindow = createWindow(graphicalWindow, showImmediately);

    if (showImmediately)
      newWindow->show();

    return graphicalWindow;
  }

  SharedPtr<Application::PlatformWindow> GraphicalApplication::createWindow(
    SharedPtr<GraphicalWindow> graphicalWindow,
    bool showImmediately
  ) {
    WindowOptions options = {};
    options.fullScreen = true;
    options.width = graphicalWindow->getWidth();
    options.height = graphicalWindow->getHeight();
    options.title = graphicalWindow->getTitle();

    SharedPtr<Application::PlatformWindow> newWindow = SharedPtr<Application::PlatformWindow>().create(options);

    if (showImmediately)
      newWindow->show();

    graphicalWindow->setNativeWindow(newWindow);

    return newWindow;
  }

  void GraphicalApplication::pollInputs() const {
    platformApplication->pollInputs();
  }

  void GraphicalApplication::onWindowShow(const WindowShown &event) {
    // Core::Application::Window &window = *windows.at(event.windowId);
    // window.resize(event.width, event.height);
  }

  void GraphicalApplication::onWindowHide(const WindowHidden &event) {

  }

  void GraphicalApplication::onWindowResize(const WindowResized &event) {

  }

  void GraphicalApplication::onWindowFocusGained(const WindowFocusGained &event) {

  }

  void GraphicalApplication::onWindowFocusLost(const WindowFocusLost &event) {

  }

  void GraphicalApplication::onWindowMinimized(const WindowMinimized &event) {

  }

  void GraphicalApplication::onWindowMaximized(const WindowMaximized &event) {

  }

  void GraphicalApplication::onWindowRestored(const WindowRestored &event) {

  }

  void GraphicalApplication::onWindowClose(const WindowClosed &event) {

  }

  void GraphicalApplication::onWindowExposed(const WindowExposed &event) {

  }

  void GraphicalApplication::onWindowMouseMotion(const WindowMouseMotion &event) {

  }

  void GraphicalApplication::onWindowMouseEnter(const WindowMouseEnter &event) {

  }

  void GraphicalApplication::onWindowMouseLeave(const WindowMouseLeave &event) {

  }

  void GraphicalApplication::onKeyPressed(const KeyPressedEvent &event) {

  }

  void GraphicalApplication::onKeyReleased(const KeyReleasedEvent &event) {

  }
}
