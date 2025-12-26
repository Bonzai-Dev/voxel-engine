#pragma once
#include <string_view>
#include <SDL3/SDL.h>

namespace Core {
  class Window {
    public:
      Window(std::string_view windowName, bool mouseLocked = true);

      ~Window();

      void Destroy() const;

      void UnlockMouse();

      void LockMouse();

      void UpdateBuffer() const;

      int GetWidth() const;

      int GetHeight() const;

    private:
      SDL_GLContext m_glContext;
      SDL_Window *m_sdlWindow;
      const SDL_DisplayMode *m_currentDisplay;
      SDL_DisplayID *m_displays;
  };
}
