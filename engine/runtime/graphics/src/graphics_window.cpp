#include <graphics/graphics_window.hpp>

namespace Engine::Application {
  GraphicsWindow::GraphicsWindow(const WindowOptions &options):
  title(options.title),
  initialWidth(options.width),
  initialHeight(options.height),
  fullscreen(options.fullScreen),
  resizable(options.resizable) {
  }

  nri::SwapChainFormat GraphicsWindow::getFormat() const {
    return this->nativeWindow->getFormat();
  }

  void GraphicsWindow::setNativeWindow(const SharedPtr<PlatformWindow> &window) {
    this->nativeWindow = window;
  }

  uint32_t GraphicsWindow::getInitialWidth() const {
    return initialWidth;
  }

  uint32_t GraphicsWindow::getInitialHeight() const {
    return initialHeight;
  }

  uint32_t GraphicsWindow::getWidth() const {
    if (!nativeWindow)
      return 0;

    return nativeWindow->getPixelWidth();
  }

  uint32_t GraphicsWindow::getHeight() const {
    if (!nativeWindow)
      return 0;

    return nativeWindow->getPixelHeight();
  }

  void GraphicsWindow::show() const {
    nativeWindow->show();
  }

  void GraphicsWindow::hide() const {
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

  SharedPtr<PlatformWindow> GraphicsWindow::getNativeWindow() const {
    return nativeWindow;
  }
}
