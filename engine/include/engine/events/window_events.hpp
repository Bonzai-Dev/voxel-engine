#pragma once
#include "event.hpp"

namespace Engine::Events {
  class WindowEvent: public Event {
    public:
      WindowEvent(EventType eventType, const char *name, uint32_t windowId): Event(eventType, name),
      windowId(windowId) {
      }

      const uint32_t windowId = 0;
  };

  class WindowShown: public WindowEvent {
    public:
      explicit WindowShown(uint32_t windowId): WindowEvent(EventType::WindowShown, "WindowShown", windowId) {
      }
  };

  class WindowHidden: public WindowEvent {
    public:
      WindowHidden(uint32_t windowId): WindowEvent(EventType::WindowHidden, "WindowHidden", windowId) {}
  };

  class WindowResized: public WindowEvent {
    public:
      WindowResized(uint32_t width, uint32_t height, uint32_t windowId) :
      WindowEvent(EventType::WindowResized, "WindowResize", windowId), width(width), height(height) {
      }

      const uint32_t width = 0;
      const uint32_t height = 0;
  };

  class WindowMouseEnter: public WindowEvent {
    public:
      WindowMouseEnter(uint32_t windowId) : WindowEvent(EventType::WindowMouseEnter, "WindowMouseEnter", windowId) {}
  };

  // Keyboard focus has been gained
  class WindowFocusGained: public WindowEvent {
    public:
      WindowFocusGained(uint32_t windowId) : WindowEvent(EventType::WindowFocusGained, "WindowFocusGained", windowId) {}
  };

  // Keyboard focus has been lost
  class WindowFocusLost: public WindowEvent {
    public:
      WindowFocusLost(uint32_t windowId) : WindowEvent(EventType::WindowFocusLost, "WindowFocusLost", windowId) {}
  };

  class WindowMinimized: public WindowEvent {
    public:
      WindowMinimized(uint32_t windowId) : WindowEvent(EventType::WindowMinimized, "WindowMinimized", windowId) {}
  };

  class WindowMaximized: public WindowEvent {
    public:
      WindowMaximized(uint32_t windowId) : WindowEvent(EventType::WindowMaximized, "WindowMaximized", windowId) {}
  };

  // Triggers when the window is restored to normal size and position after being minimized or maximized.
  class WindowRestored: public WindowEvent {
    public:
      WindowRestored(uint32_t windowId) : WindowEvent(EventType::WindowRestored, "WindowRestored", windowId) {}
  };

  class WindowClosed: public WindowEvent {
    public:
      explicit WindowClosed(uint32_t windowId) : WindowEvent(EventType::WindowClosed, "WindowClosed", windowId) {}
  };

  // Triggers when the window is exposed and should be redrawn.
  // This can occur when the window is uncovered after being hidden
  // by another window, or when it is resized.
  class WindowExposed: public WindowEvent {
    public:
      explicit WindowExposed(uint32_t windowId) : WindowEvent(EventType::WindowExposed, "WindowExposed", windowId) {}
  };
}
