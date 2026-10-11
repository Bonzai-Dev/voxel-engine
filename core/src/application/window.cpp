#include <SDL3/SDL_video.h>
#include <SDL3/SDL_mouse.h>
#include <core/events/window_events.hpp>
#include <core/memory/intrusive_ptr.hpp>
#include <core/application/logger.hpp>
#include <core/application/window.hpp>

namespace Core::Application {
  Window::Window(
     const DisplayInfo &displayInfo,
     const WindowOptions &windowOptions
   ) : options(windowOptions), displayInfo(displayInfo) {
    std::uint32_t width = options.width;
    std::uint32_t height = options.height;

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
      LOG_CORE_ERROR("Failed to create window: {}", SDL_GetError());
      return;
    }

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

  std::uint32_t Window::getId() const {
    return SDL_GetWindowID(window);
  }
}
