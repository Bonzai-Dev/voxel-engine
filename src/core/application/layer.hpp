#pragma once
#include <core/events/event.hpp>

// The application will have multiple render layers that can be rendered in a specific order.
//
// This allows better code organization and separation for each aspect of the game.
// For example, one layer could be responsible for rendering the game world, while another layer could be
// responsible for rendering the UI.

namespace Core {
  class Application;

  class Layer {
    public:
      explicit Layer(const Application &application) : application(application) {}

      virtual ~Layer() = default;

      // virtual void render();

    protected:
      const Application &application;
  };
}