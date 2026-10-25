#pragma once
#include "event.hpp"

namespace Engine::Events {
  class ApplicationQuit: public Event {
    public:
      ApplicationQuit(): Event(EventType::QuitApplication, "ApplicationQuit") {
      }
  };
}