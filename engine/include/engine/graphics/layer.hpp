#pragma once
#include "engine/application/platform_application.hpp"

// The application will have multiple render layers that can be rendered in a specific order.
//
// This allows better code organization and separation for each aspect of the game.
// For example, one layer could be responsible for rendering the game world, while another layer could be
// responsible for rendering the UI.

namespace Engine::Application {
  class PlatformApplication;
}

namespace Engine::Graphics {
  class Layer {
    public:
      explicit Layer(const Application::PlatformApplication &application) : application(application) {}

      virtual ~Layer() = default;

      // virtual void render();

    protected:
      const Application::PlatformApplication &application;
  };
}