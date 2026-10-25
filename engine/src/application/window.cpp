#include <SDL3/SDL_video.h>
#include <SDL3/SDL_mouse.h>
#include <engine/events/window_events.hpp>
#include <engine/memory/intrusive_ptr.hpp>
#include <engine/core/logging.hpp>
#include <engine/application/window.hpp>
#include "../engine.hpp"

namespace Engine::Application {
  Window::Window(
     const WindowOptions &windowOptions
   ) : options(windowOptions) {
    uint32_t width = options.width;
    uint32_t height = options.height;

    if (options.fullScreen)
      windowFlags |= SDL_WINDOW_FULLSCREEN;

    if (options.resizable)
      windowFlags |= SDL_WINDOW_RESIZABLE;

    windowFlags |= SDL_WINDOW_VULKAN;

    window = SDL_CreateWindow(
      options.windowName,
      static_cast<int>(width), static_cast<int>(height),
      windowFlags
    );

    if (!window) {
      ENGINE_LOG_ERROR(Engine::Core::coreLogger, "Failed to create window: {}", SDL_GetError());
      return;
    }

    #ifdef ENGINE_PLATFORM_WINDOWS
      ENGINE_LOG_ERROR(Engine::Core::coreLogger, "Failed to create window: {}", SDL_GetError());
    #endif

    SDL_SetWindowRelativeMouseMode(window, options.mouseLocked);
  }

  Window::~Window() {
    if (window)
      SDL_DestroyWindow(window);
  }

  void Window::close() const {
    SDL_HideWindow(window);
  }

  void Window::lockMouse() const {
    SDL_SetWindowRelativeMouseMode(window, true);
  }

  void Window::unlockMouse() const {
    SDL_SetWindowRelativeMouseMode(window, false);
  }

  uint32_t Window::getId() const {
    return SDL_GetWindowID(window);
  }
}
