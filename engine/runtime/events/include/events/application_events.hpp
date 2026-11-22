#pragma once
#include "event.hpp"

namespace Engine::Events {
  class ApplicationExitRequest: public Event {
    public:
      ApplicationExitRequest(): Event(EventType::QuitApplication, "ApplicationExitRequest") {
      }
  };
}