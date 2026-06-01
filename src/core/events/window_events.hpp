#pragma once
#include "event.hpp"

namespace Core::Events {
  class WindowEvent: public Event {
    public:
      WindowEvent(EventType eventType, const char *name, std::uint32_t windowId): Event(eventType, name),
      windowId(windowId) {
      }

      const std::uint32_t windowId = 0;
  };

  class WindowShown: public WindowEvent {
    public:
      explicit WindowShown(std::uint32_t windowId): WindowEvent(EventType::WindowShown, "WindowShown", windowId) {
      }
  };

  class WindowHidden: public WindowEvent {
    public:
      WindowHidden(std::uint32_t windowId): WindowEvent(EventType::WindowHidden, "WindowHidden", windowId) {}
  };

  class WindowResized: public WindowEvent {
    public:
      WindowResized(std::uint32_t width, std::uint32_t height, std::uint32_t windowId) :
      WindowEvent(EventType::WindowResized, "WindowResize", windowId), width(width), height(height) {
      }

      const std::uint32_t width = 0;
      const std::uint32_t height = 0;
  };

  class WindowMouseEnter: public WindowEvent {
    public:
      WindowMouseEnter(std::uint32_t windowId) : WindowEvent(EventType::WindowMouseEnter, "WindowMouseEnter", windowId) {}
  };

  // Keyboard focus has been gained
  class WindowFocusGained: public WindowEvent {
    public:
      WindowFocusGained(std::uint32_t windowId) : WindowEvent(EventType::WindowFocusGained, "WindowFocusGained", windowId) {}
  };

  // Keyboard focus has been lost
  class WindowFocusLost: public WindowEvent {
    public:
      WindowFocusLost(std::uint32_t windowId) : WindowEvent(EventType::WindowFocusLost, "WindowFocusLost", windowId) {}
  };

  class WindowMinimized: public WindowEvent {
    public:
      WindowMinimized(std::uint32_t windowId) : WindowEvent(EventType::WindowMinimized, "WindowMinimized", windowId) {}
  };

  class WindowMaximized: public WindowEvent {
    public:
      WindowMaximized(std::uint32_t windowId) : WindowEvent(EventType::WindowMaximized, "WindowMaximized", windowId) {}
  };

  // Triggers when the window is restored to normal size and position after being minimized or maximized.
  class WindowRestored: public WindowEvent {
    public:
      WindowRestored(std::uint32_t windowId) : WindowEvent(EventType::WindowRestored, "WindowRestored", windowId) {}
  };

  class WindowClosed: public WindowEvent {
    public:
      explicit WindowClosed(std::uint32_t windowId) : WindowEvent(EventType::WindowClosed, "WindowClosed", windowId) {}
  };

  // Triggers when the window is exposed and should be redrawn.
  // This can occur when the window is uncovered after being hidden
  // by another window, or when it is resized.
  class WindowExposed: public WindowEvent {
    public:
      explicit WindowExposed(std::uint32_t windowId) : WindowEvent(EventType::WindowExposed, "WindowExposed", windowId) {}
  };
}
