#pragma once
#include <engine/memory/intrusive_ptr.hpp>

#include "NRIDescs.h"
#include "Extensions/NRISwapChain.h"

struct SDL_Window;

namespace Engine::Application {
  struct WindowOptions;

  class PlatformWindow: public RefCounted {
    public:
      explicit PlatformWindow(
        const WindowOptions &windowOptions
      );

      PlatformWindow(const PlatformWindow &other) = delete;

      PlatformWindow &operator=(const PlatformWindow &other) = delete;

      PlatformWindow(PlatformWindow &&other) = delete;

      PlatformWindow &operator=(PlatformWindow &&other) = delete;

      ~PlatformWindow() override;

      void show();
      void hide();

      void minimize() const;
      void maximize() const;

      void resize(uint32_t width, uint32_t height) const;
      void restore() const;

      void *getHandle() const { return window; }

      RHI::SwapChainFormat getFormat() const;

    protected:
      SDL_Window *window = nullptr;
  };
}
