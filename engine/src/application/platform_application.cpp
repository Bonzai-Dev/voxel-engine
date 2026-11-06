#include <memory>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <engine/events/window_events.hpp>
#include <engine/events/input_events.hpp>
#include <engine/application/platform_application.hpp>
#include <engine/events/application_events.hpp>

using namespace Engine::Events;

namespace Engine::Application {
  PlatformApplication::PlatformApplication() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
      ENGINE_LOG_CRITICAL(appLogger, "Failed to initialize SDL Video: {}", SDL_GetError());
      return;
    }
  }

  PlatformApplication::~PlatformApplication() {
  }

  void PlatformApplication::getDisplayInfo(DisplayInfo &info) const {
    int count;
    SDL_DisplayID *sdlDisplays = SDL_GetDisplays(&count);

    if (!sdlDisplays) {
      ENGINE_LOG_ERROR(appLogger, "Failed to get displays: {}", SDL_GetError());
      return;
    }

    const SDL_DisplayID sdlPrimaryDisplay = SDL_GetPrimaryDisplay();

    info.monitorInfo.resize(count);
    for (size_t i = 0; i < count; ++i) {
      MonitorInfo &monitorInfo = info.monitorInfo[i];
      const SDL_DisplayID display = sdlDisplays[i];
      const SDL_DisplayMode *displayMode = SDL_GetDesktopDisplayMode(display);
      const float scale = SDL_GetDisplayContentScale(display);

      monitorInfo.name = SDL_GetDisplayName(display);
      monitorInfo.monitorId = static_cast<uint32_t>(i);
      monitorInfo.nativeWidth = static_cast<uint32_t>(displayMode->w);
      monitorInfo.nativeHeight = static_cast<uint32_t>(displayMode->h);
      monitorInfo.dpi = static_cast<uint32_t>(scale);
      monitorInfo.primaryDisplay = false;
    }

    info.primaryDisplay = &info.monitorInfo[sdlPrimaryDisplay];
    info.primaryDisplay->primaryDisplay = true;

    SDL_free(sdlDisplays);
  }

  void PlatformApplication::pollInputs() const {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          EventDispatcher::queue<ApplicationExitRequest>(ApplicationExitRequest());
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
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
          SDL_Window *window = SDL_GetWindowFromID(event.window.windowID);
          int width = 0;
          int height = 0;

          if (window && SDL_GetWindowSize(window, &width, &height)) {
            EventDispatcher::queue(WindowResized(
              static_cast<std::uint32_t>(width),
              static_cast<std::uint32_t>(height),
              event.window.windowID
            ));
          }
          break;
        }
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
