#pragma once
#include <engine/memory/intrusive_ptr.hpp>

struct SDL_Window;

namespace Engine::Application {

  struct WindowOptions {
    bool mouseLocked = false;
    bool fullScreen = false;
    bool vsync = false;
    bool resizable = true;

    uint32_t width = 800;
    uint32_t height = 600;

    const char *windowName = "Window";
  };

  class Window: public RefCounted {
    public:
      explicit Window(
        const WindowOptions &windowOptions
      );

      Window(const Window &other) = delete;

      Window &operator=(const Window &other) = delete;

      Window(Window &&other) = delete;

      Window &operator=(Window &&other) = delete;

      ~Window() override;

      unsigned long addRef(std::memory_order memoryOrder) const override {
        return ++referenceCount;
      }

      unsigned long release(std::memory_order memoryOrder) const override {
        unsigned long result = --referenceCount;
        if (result == 0) {
          delete this;
        }
        return result;
      }

      const WindowOptions &getOptions() const { return options; }

      void show() { shown = true; }

      void hide() { shown = false; }

      void focusMouse() { mouseFocused = true; }

      void unfocusMouse() { mouseFocused = false; }

      void focusKeyboard() { keyboardFocused = true; }

      void unfocusKeyboard() { keyboardFocused = false; }

      void minimize() { minimized = true; }

      void maximize() { minimized = false; }

      void resize(uint32_t width, uint32_t height) {
        options.width = width;
        options.height = height;
      }

      void restore() { minimized = false; }

      void close() const;

      void lockMouse() const;

      void unlockMouse() const;

      uint32_t getId() const;

      void *getHandle() const { return window; }

    protected:
      bool minimized = false;
      bool shown = false;
      bool mouseFocused = false; // Whether if the mouse is in the window or not
      bool keyboardFocused = false;

      WindowOptions options;

      SDL_Window *window = nullptr;
      uint64_t windowFlags = 0;
  };
}
