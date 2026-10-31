#pragma once
#include <engine/application/application.hpp>
#include <engine/memory/intrusive_ptr.hpp>

namespace Engine::Graphics {
  class GraphicalApplication {
    public:
      GraphicalApplication();

      virtual ~GraphicalApplication() = default;

      virtual IntrusivePtr<Application::Window> createWindow(bool showImmediately = false) = 0;

    private:

      // Application::Application application = nullptr;
  };
}