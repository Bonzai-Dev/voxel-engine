#include <SDL3/SDL.h>
#include <core/application/logger.hpp>
#include <core/events/application_events.hpp>
#include <core/events/window_events.hpp>
#include <core/events/input_events.hpp>
#include <core/renderer/renderer.hpp>
#include <core/rhi/vulkan/vulkan_rendering_device.hpp>
#include <core/rhi/vulkan/vulkan_window.hpp>
#include "application.hpp"

namespace Core {
  using namespace Events;

  Application::Application(const char *name) : name(name) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
      LOG_CORE_CRITICAL("Failed to initialize SDL Video: {}", SDL_GetError());
      return;
    }

    displays = SDL_GetDisplays(&displayCount);
    if (displayCount > 0)
      LOG_CORE_INFO("Found {} display(s)", displayCount);
    else
      LOG_CORE_WARNING("No displays found");

    currentDisplay = SDL_GetCurrentDisplayMode(displays[0]);
    displayInfo = {
      .displayId = currentDisplay->displayID,
      .width = static_cast<std::uint32_t>(currentDisplay->w),
      .height = static_cast<std::uint32_t>(currentDisplay->h)
    };

    EventDispatcher::listen<ApplicationQuit>([&](const ApplicationQuit &event) { quit(); });

    EventDispatcher::listen<WindowShown>([&](const WindowShown &event) { onWindowShow(event); });
    EventDispatcher::listen<WindowHidden>([&](const WindowHidden &event) { onWindowHide(event); });
    EventDispatcher::listen<WindowResized>([&](const WindowResized &event) { onWindowResize(event); });

    EventDispatcher::listen<WindowMouseEnter>([&](const WindowMouseEnter &event) { onWindowMouseEnter(event); });
    EventDispatcher::listen<WindowMouseLeave>([&](const WindowMouseLeave &event) { onWindowMouseLeave(event); });
    EventDispatcher::listen<WindowMouseMotion>([&](const WindowMouseMotion &event) { onWindowMouseMotion(event); });

    EventDispatcher::listen<WindowFocusGained>([&](const WindowFocusGained &event) { onWindowFocusGained(event); });
    EventDispatcher::listen<WindowFocusLost>([&](const WindowFocusLost &event) { onWindowFocusLost(event); });
    EventDispatcher::listen<WindowMinimized>([&](const WindowMinimized &event) { onWindowMinimized(event); });
    EventDispatcher::listen<WindowMaximized>([&](const WindowMaximized &event) { onWindowMaximized(event); });
    EventDispatcher::listen<WindowRestored>([&](const WindowRestored &event) { onWindowRestored(event); });
    EventDispatcher::listen<WindowClosed>([&](const WindowClosed &event) { onWindowClose(event); });
    EventDispatcher::listen<WindowExposed>([&](const WindowExposed &event) { onWindowExposed(event); });

    EventDispatcher::listen<KeyPressedEvent>([&](const KeyPressedEvent &event) { onKeyPressed(event); });
    EventDispatcher::listen<KeyReleasedEvent>([&](const KeyReleasedEvent &event) { onKeyReleased(event); });

    switch (graphicsBackend) {
      case Graphics::Backend::Vulkan:
        renderingDevice = std::make_unique<Graphics::VulkanRenderingDevice>(graphicsBackend, name, displayInfo);
        break;

      case Graphics::Backend::None:
        LOG_CORE_CRITICAL("No graphics backend selected. Select a backend to render graphics");
        break;

      default:
        LOG_CORE_CRITICAL("Unsupported graphics backend selected");
        break;
    }
  }

  Application::~Application() {
    quit();
  }

  Graphics::Backend Application::selectGraphicsBackend() const {
    // TODO: Implement graphics backend selection based on platform and availability
    return Graphics::Backend::Vulkan;
  }

  void Application::run() const {
    static std::uint64_t lastFrameTime = 0;
    static std::uint64_t currentFrameTime = 0;
    while (running) {
      lastFrameTime = currentFrameTime;
      currentFrameTime = SDL_GetPerformanceCounter();
      deltaTime = currentFrameTime - lastFrameTime;

      eventDispatcher.process();
      pollInputs();
    }
  }

  void Application::createWindow(const WindowOptions &options) const {
    RefCountedPtr<Graphics::VulkanWindow> window = RefCountedPtr<Graphics::VulkanWindow>::create(
      renderingDevice->getContext(),
      displayInfo,
      options
    );
    windows.emplace(window->getId(), window);
  }

  void Application::quit() const {
    if (!running)
      return;

    // TODO: remove listeners?
    running = false;
    SDL_free(displays);
    SDL_Quit();
  }

  void Application::pollInputs() const {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          EventDispatcher::queue<ApplicationQuit>(ApplicationQuit());
          break;
        case SDL_EVENT_WINDOW_SHOWN:
          EventDispatcher::queue(WindowShown(event.window.windowID));
          break;
        case SDL_EVENT_WINDOW_HIDDEN:
          EventDispatcher::queue(WindowHidden(event.window.windowID));
          break;
        case SDL_EVENT_WINDOW_RESIZED:
          EventDispatcher::queue(WindowResized(
            event.window.data1, event.window.data2,
            event.window.windowID
          ));
          break;
        case SDL_EVENT_WINDOW_EXPOSED:
          EventDispatcher::queue(WindowExposed(event.window.windowID));
          break;
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
          EventDispatcher::queue(WindowMouseEnter(event.window.windowID));
          break;
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
          EventDispatcher::queue(WindowMouseLeave(event.window.windowID));
          break;
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
          EventDispatcher::queue(WindowFocusGained(event.window.windowID));
          break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
          EventDispatcher::queue(WindowFocusLost(event.window.windowID));
          break;
        case SDL_EVENT_WINDOW_MINIMIZED:
          EventDispatcher::queue(WindowMinimized(event.window.windowID));
          break;
        case SDL_EVENT_WINDOW_MAXIMIZED:
          EventDispatcher::queue(WindowMaximized(event.window.windowID));
          break;
        case SDL_EVENT_WINDOW_RESTORED:
          EventDispatcher::queue(WindowRestored(event.window.windowID));
          break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
          EventDispatcher::queue(WindowClosed(event.window.windowID));
          break;
        case SDL_EVENT_MOUSE_MOTION:
          EventDispatcher::queue(WindowMouseMotion(
            event.motion.xrel, event.motion.yrel,
            event.motion.x, event.motion.y,
            event.motion.windowID
          ));
          break;
        case SDL_EVENT_KEY_DOWN:
          // TODO: Take into account scancodes
          // https://wiki.libsdl.org/SDL3/BestKeyboardPractices
          EventDispatcher::queue(KeyPressedEvent(event.key.key, event.key.repeat, event.key.windowID));
          break;
        case SDL_EVENT_KEY_UP:
          EventDispatcher::queue(KeyReleasedEvent(event.key.key, event.key.windowID));
          break;
        default:
          break;
      }
    }
  }
}
