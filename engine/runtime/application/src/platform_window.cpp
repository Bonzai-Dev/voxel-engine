#include <SDL3/SDL_video.h>
#include <SDL3/SDL_mouse.h>
#include <application/platform_window.hpp>
#include "Extensions/NRISwapChain.h"
#include <core/logging.hpp>
#include "application.hpp"

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
      ENGINE_LOG_ERROR(applicationLogger, "Failed to create window: {}", SDL_GetError());
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
    SDL_SyncWindow(window);
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

  float PlatformWindow::getDPIScaleFactor() const {
    return SDL_GetDisplayContentScale(SDL_GetWindowID(window));
  }

  uint32_t PlatformWindow::getPixelWidth() const {
    int width;
    SDL_GetWindowSizeInPixels(window, &width, nullptr);
    return static_cast<uint32_t>(width);
  }

  uint32_t PlatformWindow::getPixelHeight() const {
    int height;
    SDL_GetWindowSizeInPixels(window, nullptr, &height);
    return static_cast<uint32_t>(height);
  }

  uint32_t PlatformWindow::getWindowWidth() const {
    int width;
    SDL_GetWindowSize(window, &width, nullptr);
    return static_cast<uint32_t>(width);
  }

  uint32_t PlatformWindow::getWindowHeight() const {
    int height;
    SDL_GetWindowSize(window, nullptr, &height);
    return static_cast<uint32_t>(height);
  }
}
