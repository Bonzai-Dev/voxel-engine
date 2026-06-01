#pragma once
#include "window_events.hpp"
#include "event.hpp"

namespace Core::Events {
  class KeyEvent: public WindowEvent {
    public:
      KeyEvent(std::uint32_t keyCode, EventType eventType, const char *name, std::uint64_t windowId):
      WindowEvent(eventType, name, windowId), keyCode(keyCode) {
      }

      const std::uint32_t keyCode;
  };

  class KeyPressedEvent: public KeyEvent {
    public:
      KeyPressedEvent(std::uint32_t keyCode, bool repeated, std::uint64_t windowId):
      KeyEvent(keyCode, EventType::KeyPressed, "KeyPressed", windowId), repeated(repeated) {
      }

      // Whether the key press has been repeated due to the user holding down the key.
      const bool repeated;
  };

  class KeyReleasedEvent: public KeyEvent {
    public:
      KeyReleasedEvent(std::uint32_t keyCode, std::uint64_t windowId):
      KeyEvent(keyCode, EventType::KeyReleased, "KeyReleased", windowId) {
      }
  };

  class WindowMouseLeave: public WindowEvent {
    public:
      WindowMouseLeave(std::uint32_t windowId) : WindowEvent(EventType::WindowMouseEnter, "WindowMouseLeave", windowId) {}
  };

  class WindowMouseMotion: public WindowEvent {
    public:
      WindowMouseMotion(float deltaX, float deltaY, float positionX, float positionY, std::uint32_t windowId) :
      WindowEvent(EventType::MouseMotion, "MouseMotion", windowId),
      x(positionX), y(positionY), deltaX(deltaX), deltaY(deltaY) {
      }

      const float x = 0;
      const float y = 0;

      const float deltaX = 0;
      const float deltaY = 0;
  };
}