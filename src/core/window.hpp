#pragma once
#include <string_view>
#include <SDL3/SDL.h>

class Window {
  public:
    Window(std::string_view windowName);

    void Destroy() const;

    void UpdateBuffer() const;

  private:
    SDL_GLContext m_glContext;
    SDL_Window* m_sdlWindow;
    SDL_DisplayMode m_displayMode;
};
