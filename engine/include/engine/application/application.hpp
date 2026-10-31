#pragma once
#include <memory>
#include <engine/application/layer.hpp>
#include <engine/events/event.hpp>
#include <engine/events/application_events.hpp>
#include <engine/events/event.hpp>
#include <engine/events/window_events.hpp>
#include <engine/events/input_events.hpp>
#include <engine/application/window.hpp>
#include <engine/core/logging.hpp>
#include <engine/renderer/renderer.hpp>
#include "application.hpp"

namespace Engine::Application {
  ENGINE_DEFINE_LOG_CATEGORY(appLogger, "application");

  struct MonitorInfo {
    uint32_t monitorId = 0;
    uint32_t nativeWidth = 0;
    uint32_t nativeHeight = 0;
    uint32_t dpi = 0;
    bool primaryDisplay = false;
  };

  class Application {
    public:
      explicit Application(const char *name);

      Application(const Application &other) = delete;

      Application &operator=(const Application &other) = delete;

      Application(Application &&other) = delete;

      Application &operator=(Application &&other) = delete;

      ~Application();

      template <typename LayerT>
      requires(std::is_base_of_v<Layer, LayerT>)
      void addLayer() const {
        layers.push_back(std::make_unique<LayerT>(*this));
      }

      void createWindow(const WindowOptions &options) const;

      void quit() const;

      void run() const;

    private:
      void onWindowShow(const Events::WindowShown &event) {
        // windows.at(event.windowId)->show();
      }

      void onWindowHide(const Events::WindowHidden &event) {
        // windows.at(event.windowId)->hide();
      }

      void onWindowResize(const Events::WindowResized &event) {
        // Core::Application::Window &window = *windows.at(event.windowId);
        // window.resize(event.width, event.height);
        // LOG_CORE_INFO("Resize event processed and resized to {}x{}", event.width, event.height);
      }

      void onWindowMouseMotion(const Events::WindowMouseMotion &event) {
      }

      void onWindowMouseEnter(const Events::WindowMouseEnter &event) {
      }

      void onWindowMouseLeave(const Events::WindowMouseLeave &event) {
      }

      void onWindowFocusGained(const Events::WindowFocusGained &event) {
      }

      void onWindowFocusLost(const Events::WindowFocusLost &event) {
      }

      void onWindowMinimized(const Events::WindowMinimized &event) {
      }

      void onWindowMaximized(const Events::WindowMaximized &event) {
        Window &window = *windows.at(event.windowId);
        window.maximize();
        if (renderer) {
          // renderer->recreateWindowSurface(window);
        }
      }

      void onWindowRestored(const Events::WindowRestored &event) {
        Window &window = *windows.at(event.windowId);
        window.restore();
        if (renderer) {
          // renderer->recreateWindowSurface(window);
        }
      }

      void onWindowClose(const Events::WindowClosed &event) {
      }

      void onWindowExposed(const Events::WindowExposed &event) {
      }

      void onKeyPressed(const Events::KeyPressedEvent &event) {
      }

      void onKeyReleased(const Events::KeyReleasedEvent &event) {
      }

      void pollInputs() const;

      mutable std::unique_ptr<Renderer::Renderer> renderer;

      const char *name;
      mutable Events::EventDispatcher eventDispatcher;

      mutable std::vector<std::unique_ptr<Layer>> layers;

      mutable bool running = true;

      MonitorInfo displayInfo = {};

      int displayCount = 0;
      mutable SDL_DisplayID *displays = nullptr;
      const SDL_DisplayMode *currentDisplay = nullptr;
      mutable std::unordered_map<std::uint32_t, std::unique_ptr<Window>> windows;
  };
}
