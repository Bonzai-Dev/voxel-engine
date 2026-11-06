#include <engine/graphics/graphical_window.hpp>

namespace Engine::Graphics {
  GraphicalWindow::GraphicalWindow(WindowOptions options):
  width(options.width),
  height(options.height),
  title(options.title),
  fullscreen(options.fullScreen) {

  }

  void GraphicalWindow::setNativeWindow(SharedPtr<Application::PlatformWindow> window) {
    this->nativeWindow = window;
  }
}
