#include <glad/gl.h>
#include <iostream>
#include <string_view>
#include <SDL3/SDL.h>
#include "../logger.hpp"
#include "window.hpp"

using namespace Logger;

namespace Core {
  Window::Window(const std::string_view windowName, bool mouseLocked) {
    static bool sdlInitialized = false;
    if (!sdlInitialized) {
      if (!SDL_Init(SDL_INIT_VIDEO)) {
        logError(Context::Core, "Failed to initialize SDL: %s", SDL_GetError());
        return;
      }
      sdlInitialized = true;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Get display information
    int displayCount;
    displays = SDL_GetDisplays(&displayCount);
    if (displayCount > 0) {
      logInfo(Context::Core, "Found %d display(s).", displayCount);
    } else {
      logWarning(Context::Core, "No display found.");
    }
    currentDisplay = SDL_GetCurrentDisplayMode(displays[0]);

    // Creating window
    sdlWindow = SDL_CreateWindow(
      windowName.data(),
      currentDisplay->w, currentDisplay->h,
      SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN
    );

    if (!sdlWindow) {
      logError(Context::Core, "Failed to create SDL window: %s", SDL_GetError());
      return;
    }

    // Creating OpenGL context
    glContext = SDL_GL_CreateContext(sdlWindow);
    const int version = gladLoadGL(SDL_GL_GetProcAddress);
    if (!version) {
      logError(Context::Renderer, "Failed to initialize OpenGL context.");
      return;
    }

    SDL_SetWindowRelativeMouseMode(sdlWindow, mouseLocked);
    logInfo(
      Context::Renderer,
      "Program has successfully initialized OpenGL %d.%d core profile.",
      GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version)
    );
  }

  Window::~Window() {
    destroy();
  }

  void Window::destroy() const {
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(sdlWindow);
    SDL_free(displays);
  }

  int Window::getWidth() const {
    return currentDisplay->w;
  }

  int Window::getHeight() const {
    return currentDisplay->h;
  }

  void Window::updateBuffer() const {
    SDL_GL_SwapWindow(sdlWindow);
  }

  void Window::unlockMouse() {
    SDL_SetWindowRelativeMouseMode(sdlWindow, false);
  }

  void Window::lockMouse() {
    SDL_SetWindowRelativeMouseMode(sdlWindow, true);
  }
}
