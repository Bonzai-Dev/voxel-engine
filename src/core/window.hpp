#pragma once
#include <string_view>
#include <SDL3/SDL.h>

class Window {
  public:
    Window(std::string_view windowName, bool mouseLocked = true);

    void Destroy() const;

    void UnlockMouse();

    void LockMouse();

    bool running;

    double deltaTime = 0;

  private:
    void Render();

    void UpdateBuffer() const;

    SDL_GLContext m_glContext;
    SDL_Window *m_sdlWindow;
    const SDL_DisplayMode *m_currentDisplay;
    SDL_DisplayID *m_displays;
    double m_lastFrameTime = 0;
};
