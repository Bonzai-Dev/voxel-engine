#include <SDL3/SDL_video.h>
#include <SDL3/SDL_mouse.h>
#include <core/events/window_events.hpp>
#include "window.hpp"
#include "logger.hpp"
#include "core/rhi/vulkan/vulkan_rendering_device.hpp"

namespace Core {
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
    SDL_DestroyWindow(window);
  }
}
