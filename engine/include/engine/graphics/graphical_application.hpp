#pragma once
#include <engine/memory/intrusive_ptr.hpp>
#include <engine/graphics/layer.hpp>
#include <engine/events/event.hpp>
#include "graphical_window.hpp"

namespace Engine::Events {
  class WindowShown;
  class WindowHidden;
  class WindowResized;
  class WindowFocusGained;
  class WindowFocusLost;
  class WindowMinimized;
  class WindowMaximized;
  class WindowRestored;
  class WindowClosed;
  class WindowExposed;
  class WindowMouseMotion;
  class WindowMouseEnter;
  class WindowMouseLeave;

  class KeyPressedEvent;
  class KeyReleasedEvent;
}

namespace Engine::Application {
  class PlatformWindow;
  class PlatformApplication;
}

namespace Engine::Graphics {
  class GraphicalApplication {
    public:
      GraphicalApplication();

      virtual ~GraphicalApplication() = default;

      template <typename LayerT>
      requires(std::is_base_of_v<Layer, LayerT>)
      void addLayer() const {
        layers.push_back(std::make_unique<LayerT>(*this));
      }

      virtual SharedPtr<GraphicalWindow> addWindow(SharedPtr<GraphicalWindow> graphicalWindow, bool showImmediately);

      void pollInputs() const;

    protected:
      SharedPtr<Application::PlatformApplication> platformApplication = nullptr;

    private:
      SharedPtr<Application::PlatformWindow> createWindow(SharedPtr<GraphicalWindow> graphicalWindow, bool showImmediately);

      mutable std::vector<std::unique_ptr<Layer>> layers;

      mutable Events::EventDispatcher eventDispatcher;

      void onWindowShow(const Events::WindowShown &event);
      void onWindowHide(const Events::WindowHidden &event);
      void onWindowResize(const Events::WindowResized &event);
      void onWindowFocusGained(const Events::WindowFocusGained &event);
      void onWindowFocusLost(const Events::WindowFocusLost &event);
      void onWindowMinimized(const Events::WindowMinimized &event);
      void onWindowMaximized(const Events::WindowMaximized &event);
      void onWindowRestored(const Events::WindowRestored &event);
      void onWindowClose(const Events::WindowClosed &event);
      void onWindowExposed(const Events::WindowExposed &event);
      void onWindowMouseMotion(const Events::WindowMouseMotion &event);
      void onWindowMouseEnter(const Events::WindowMouseEnter &event);
      void onWindowMouseLeave(const Events::WindowMouseLeave &event);

      void onKeyPressed(const Events::KeyPressedEvent &event);
      void onKeyReleased(const Events::KeyReleasedEvent &event);
  };
}
