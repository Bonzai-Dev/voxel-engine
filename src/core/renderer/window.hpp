#pragma once
#include <string_view>
#include <SDL3/SDL.h>

namespace Renderer {
  class Window {
    public:
      Window(std::string_view windowName, bool mouseLocked = true);

      ~Window();

      void destroy() const;

      void unlockMouse();

      void lockMouse();

      void updateBuffer() const;

      int getWidth() const;

      int getHeight() const;

    private:
      SDL_GLContext glContext;
      SDL_Window *sdlWindow;
      const SDL_DisplayMode *currentDisplay;
      SDL_DisplayID *displays;
  };
}
