#include <glad/gl.h>
#include <string_view>
#include <SDL3/SDL.h>
#include <core/logger.hpp>
#include "window.hpp"

using namespace logger;

Window::Window(const std::string_view windowName) {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  m_sdlWindow = SDL_CreateWindow(
    windowName.data(),
    1920, 1080,
    SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN
  );
  m_glContext = SDL_GL_CreateContext(m_sdlWindow);

  const int version = gladLoadGL(SDL_GL_GetProcAddress);
  if (!version) {
    logError(Context::RENDERER, "Failed to initialize OpenGL context.");
    return;
  }

  logInfo(
    Context::RENDERER,
    "Program has successfully initialized OpenGL %d.%d core profile.",
    GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version)
  );
}

void Window::Destroy() const {
  SDL_GL_DestroyContext(m_glContext);
  SDL_DestroyWindow(m_sdlWindow);
}

void Window::UpdateBuffer() const {
  SDL_GL_SwapWindow(m_sdlWindow);
}
