#include <engine/application/graphics_window.hpp>

namespace Engine::Application {
  GraphicsWindow::GraphicsWindow(const WindowOptions &options):
  title(options.title),
  width(options.width),
  height(options.height),
  fullscreen(options.fullScreen),
  resizable(options.resizable) {
  }

  RHI::SwapChainFormat GraphicsWindow::getFormat() const {
    return this->nativeWindow->getFormat();
  }

  void GraphicsWindow::setNativeWindow(const SharedPtr<Application::PlatformWindow> &window) {
    this->nativeWindow = window;
  }

  void GraphicsWindow::show() {
    nativeWindow->show();
  }

  void GraphicsWindow::hide() {
    nativeWindow->hide();
  }

  void GraphicsWindow::minimize() const {
    nativeWindow->minimize();
  }

  void GraphicsWindow::maximize() const {
    nativeWindow->maximize();
  }

  void GraphicsWindow::resize(uint32_t width, uint32_t height) const {
    nativeWindow->resize(width, height);
  }

  void GraphicsWindow::restore() const {
    nativeWindow->restore();
  }

  SharedPtr<Application::PlatformWindow> GraphicsWindow::getNativeWindow() const {
    return nativeWindow;
  }
}
