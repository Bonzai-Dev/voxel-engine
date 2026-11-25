#pragma once
#include <cstdint>
#include <string>
#include <core/shared_pointer.hpp>
#include <application/platform_window.hpp>
#include "NRIDescs.h"

namespace Engine::Application {
  class GraphicsWindow: public RefCounted {
    public:
      explicit GraphicsWindow(const WindowOptions& options);

      void setNativeWindow(const SharedPtr<PlatformWindow> &nativeWindow);

      SharedPtr<PlatformWindow> getNativeWindow() const;

      const std::string &getTitle() const { return title; }

      uint32_t getInitialWidth() const;
      uint32_t getInitialHeight() const;

      uint32_t getWidth() const;
      uint32_t getHeight() const;

      bool isFullscreen() const { return fullscreen; }
      bool isResizable() const { return resizable; }
      bool windowShownBefore() const { return windowShownOnce; }

      nri::SwapChainFormat getFormat() const;

      /* Makes window visible */
      void show() const;

      /* Makes window invisible */
      void hide() const;

      void minimize() const;
      void maximize() const;

      void resize(uint32_t width, uint32_t height) const;

      /* Request that the size and position of a minimized or maximized window be restored */
      void restore() const;

    private:
      SharedPtr<PlatformWindow> nativeWindow;

      std::string title;

      const uint32_t initialWidth;
      const uint32_t initialHeight;

      bool fullscreen;
      bool resizable;

      bool windowShownOnce = false;
  };
}
