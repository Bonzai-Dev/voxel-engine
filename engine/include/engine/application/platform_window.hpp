#pragma once
#include <engine/memory/intrusive_ptr.hpp>

namespace Engine::Graphics {
  struct WindowOptions;
}

struct SDL_Window;

namespace Engine::Application {
  class PlatformWindow: public RefCounted {
    public:
      explicit PlatformWindow(
        const Graphics::WindowOptions &windowOptions
      );

      PlatformWindow(const PlatformWindow &other) = delete;

      PlatformWindow &operator=(const PlatformWindow &other) = delete;

      PlatformWindow(PlatformWindow &&other) = delete;

      PlatformWindow &operator=(PlatformWindow &&other) = delete;

      ~PlatformWindow() override;

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

      void show();
      void hide();

      void minimize() const;
      void maximize() const;

      void resize(uint32_t width, uint32_t height) const;
      void restore() const;

      void close() const;

      void *getHandle() const { return window; }

    protected:
      SDL_Window *window = nullptr;
  };
}
