#pragma once
#include <string>
#include <engine/memory/intrusive_ptr.hpp>
#include <engine/application/platform_window.hpp>

#include "NRIDescs.h"

namespace Engine::Application {
  struct WindowOptions {
    std::string title = "Window";

    uint32_t width = 800;
    uint32_t height = 600;

    bool focusWhenFirstShown = false;
    bool fullScreen = false;
    bool resizable = true;
  };

  class GraphicsWindow: public RefCounted {
    public:
      explicit GraphicsWindow(const WindowOptions& options);

      void setNativeWindow(const SharedPtr<PlatformWindow> &nativeWindow);

      SharedPtr<PlatformWindow> getNativeWindow() const;

      const std::string &getTitle() const { return title; }

      uint32_t getWidth() const { return width; }
      uint32_t getHeight() const { return height; }

      bool isFullscreen() const { return fullscreen; }
      bool isResizable() const { return resizable; }
      bool windowShownBefore() const { return windowShownOnce; }

      RHI::SwapChainFormat getFormat() const;

      /* Makes window visible */
      void show();

      /* Makes window invisible */
      void hide();

      void minimize() const;
      void maximize() const;

      void resize(uint32_t width, uint32_t height) const;

      /* Request that the size and position of a minimized or maximized window be restored */
      void restore() const;

    private:
      SharedPtr<PlatformWindow> nativeWindow;

      std::string title;

      uint32_t width;
      uint32_t height;

      bool fullscreen;
      bool resizable;

      bool windowShownOnce = false;
  };
}
