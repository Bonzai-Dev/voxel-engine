#include <SDL3/SDL_video.h>
#include <SDL3/SDL_mouse.h>
#include <engine/application/graphics_window.hpp>
#include <engine/memory/intrusive_ptr.hpp>
#include <engine/core/logging.hpp>
#include <engine/application/platform_window.hpp>
#include "../engine.hpp"
#include "Extensions/NRISwapChain.h"

namespace Engine::Application {
  PlatformWindow::PlatformWindow(
    const WindowOptions &windowOptions
  ) {
    uint64_t windowFlags = SDL_WINDOW_HIDDEN;

    if (windowOptions.fullScreen)
      windowFlags |= SDL_WINDOW_FULLSCREEN;

    if (windowOptions.resizable)
      windowFlags |= SDL_WINDOW_RESIZABLE;

    if (windowOptions.focusWhenFirstShown)
      windowFlags |= SDL_WINDOW_INPUT_FOCUS;

    windowFlags |= SDL_WINDOW_VULKAN;

    window = SDL_CreateWindow(
      windowOptions.title.c_str(),
      static_cast<int>(windowOptions.width), static_cast<int>(windowOptions.height),
      windowFlags
    );

    if (!window) {
      ENGINE_LOG_ERROR(Engine::Core::coreLogger, "Failed to create window: {}", SDL_GetError());
      return;
    }

    if (windowOptions.focusWhenFirstShown)
      SDL_RaiseWindow(window);
  }

  RHI::SwapChainFormat PlatformWindow::getFormat() const {
    return RHI::SwapChainFormat::BT709_G22_8BIT;
  }

  PlatformWindow::~PlatformWindow() {
    if (window)
      SDL_DestroyWindow(window);
  }

  void PlatformWindow::show() {
    SDL_ShowWindow(window);
  }

  void PlatformWindow::hide() {
    SDL_HideWindow(window);
  }

  void PlatformWindow::minimize() const {
    SDL_MinimizeWindow(window);
  }

  void PlatformWindow::maximize() const {
    SDL_MaximizeWindow(window);
  }

  void PlatformWindow::resize(uint32_t width, uint32_t height) const {
    SDL_SetWindowSize(window, static_cast<int>(width), static_cast<int>(height));
  }

  void PlatformWindow::restore() const {
    SDL_RestoreWindow(window);
  }
}
