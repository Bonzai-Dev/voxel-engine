#include <glad/gl.h>
#include <iostream>
#include <string_view>
#include <SDL3/SDL.h>
#include "logger.hpp"
#include "renderer/renderer.hpp"
#include "window.hpp"

using namespace Logger;

namespace Core {
  Window::Window(const std::string_view windowName, bool mouseLocked) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Get display information
    int displayCount;
    m_displays = SDL_GetDisplays(&displayCount);
    if (displayCount > 0) {
      logInfo(Context::CORE, "Found %d display(s).", displayCount);
    } else {
      logWarning(Context::CORE, "No display found.");
    }
    m_currentDisplay = SDL_GetCurrentDisplayMode(m_displays[0]);

    // Creating window
    m_sdlWindow = SDL_CreateWindow(
      windowName.data(),
      m_currentDisplay->w, m_currentDisplay->h,
      SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN
    );

    if (!m_sdlWindow) {
      logError(Context::CORE, "Failed to create SDL window: %s", SDL_GetError());
      return;
    }

    // Creating OpenGL context
    m_glContext = SDL_GL_CreateContext(m_sdlWindow);
    const int version = gladLoadGL(SDL_GL_GetProcAddress);
    if (!version) {
      logError(Context::RENDERER, "Failed to initialize OpenGL context.");
      return;
    }

    SDL_SetWindowRelativeMouseMode(m_sdlWindow, mouseLocked);
    logInfo(
      Context::RENDERER,
      "Program has successfully initialized OpenGL %d.%d core profile.",
      GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version)
    );
  }

  Window::~Window() {
    Destroy();
  }

  void Window::Destroy() const {
    SDL_GL_DestroyContext(m_glContext);
    SDL_DestroyWindow(m_sdlWindow);
    SDL_free(m_displays);
  }

  void Window::UpdateBuffer() const {
    SDL_GL_SwapWindow(m_sdlWindow);
  }

  void Window::UnlockMouse() {
    SDL_SetWindowRelativeMouseMode(m_sdlWindow, false);
  }

  void Window::LockMouse() {
    SDL_SetWindowRelativeMouseMode(m_sdlWindow, true);
  }
}
