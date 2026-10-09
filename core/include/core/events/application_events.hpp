#pragma once
#include "event.hpp"

namespace Core::Events {
  class ApplicationQuit: public Event {
    public:
      ApplicationQuit(): Event(EventType::QuitApplication, "ApplicationQuit") {
      }
  };
}