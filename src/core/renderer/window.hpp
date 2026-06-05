#pragma once
#include <cstdint>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_mouse.h>
#include <core/events/window_events.hpp>

namespace Core::Graphics {
  struct WindowOptions {
    bool mouseLocked = false;
    bool fullScreen = false;
    bool vsync = false;
    bool resizable = true;

    std::uint32_t width = 800;
    std::uint32_t height = 600;

    const char *windowName = "Window";
  };

  struct DisplayInfo {
    std::uint32_t displayId;

    std::uint32_t width;
    std::uint32_t height;
  };

  class Window {
    public:
      explicit Window(
        const DisplayInfo &displayInfo,
        const WindowOptions &windowOptions
      );

      WindowOptions options;

      Window(const Window &other) = delete;

      Window &operator=(const Window &other) = delete;

      Window(Window &&other) noexcept;

      Window &operator=(Window &&other) = delete;

      virtual ~Window();

      virtual void render() const {}

      void show() { shown = true; }

      void hide() { shown = false; }

      void focusMouse() { mouseFocused = true; }

      void unfocusMouse() { mouseFocused = false; }

      void focusKeyboard() { keyboardFocused = true; }

      void unfocusKeyboard() { keyboardFocused = false; }

      void minimize() { minimized = true; }

      void maximize() { minimized = false; }

      void resize(std::uint32_t width, std::uint32_t height) {
        options.width = width;
        options.height = height;
      }

      void restore() { minimized = false; }

      void close() const { SDL_HideWindow(window); }

      void lockMouse() const { SDL_SetWindowRelativeMouseMode(window, true); }

      void unlockMouse() const { SDL_SetWindowRelativeMouseMode(window, false); }

      std::uint32_t getId() const { return SDL_GetWindowID(window); }

    protected:
      bool minimized = false;
      bool shown = false;
      bool mouseFocused = false; // Whether if the mouse is in the window or not
      bool keyboardFocused = false;

      const DisplayInfo &displayInfo;

      SDL_Window *window = nullptr;
      std::uint64_t windowFlags = 0;
  };
}