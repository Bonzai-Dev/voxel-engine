#pragma once
#include <string>
#include <engine/memory/intrusive_ptr.hpp>
#include <engine/application/platform_window.hpp>

namespace Engine::Graphics {
  struct WindowOptions {
    bool focusWhenFirstShown = false;
    bool fullScreen = false;
    bool resizable = true;

    uint32_t width = 800;
    uint32_t height = 600;

    std::string title = "Window";
  };

  class GraphicalWindow: RefCounted {
    public:
      GraphicalWindow(WindowOptions options);

      unsigned long addRef() const override {
        return ++referenceCount;
      }

      unsigned long release() const override {
        unsigned long result = --referenceCount;
        if (result == 0) {
          delete this;
        }
        return result;
      }

      void setNativeWindow(SharedPtr<Application::PlatformWindow> nativeWindow);

      const std::string &getTitle() const { return title; }

      uint32_t getWidth() const { return width; }
      uint32_t getHeight() const { return height; }

    private:
      SharedPtr<Application::PlatformWindow> nativeWindow;

      std::string title;

      uint32_t width;
      uint32_t height;

      bool fullscreen;
  };
}
