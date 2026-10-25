#pragma once
#include <memory>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_mouse.h>
#include <engine/application/layer.hpp>
#include <engine/events/event.hpp>
#include <engine/events/application_events.hpp>
#include <engine/events/event.hpp>
#include <engine/events/window_events.hpp>
#include <engine/events/input_events.hpp>
#include <engine/application/window.hpp>
#include <engine/core/logging.hpp>
#include <engine/renderer/renderer.hpp>
#include "engine/application/application.hpp"

namespace Application {
  ENGINE_DEFINE_LOG_CATEGORY(appLogger, "APPLICATION");

  class Application {
    public:
      explicit Application(const char *name);

      Application(const Application &other) = delete;

      Application &operator=(const Application &other) = delete;

      Application(Application &&other) = delete;

      Application &operator=(Application &&other) = delete;

      ~Application();

#ifdef NDEBUG
      static constexpr bool debugEnabled = false;
#else
      static constexpr bool debugEnabled = true;
#endif

      template <typename LayerT>
        requires(std::is_base_of_v<Engine::Application::Layer, LayerT>)
      void addLayer() const {
        layers.push_back(std::make_unique<LayerT>(*this));
      }

      void createWindow(const Engine::Application::WindowOptions &options) const;

      const std::uint64_t &getDeltaTime() const { return deltaTime; }

      void quit() const;

      void run() const;

    private:
      void onWindowShow(const Engine::Events::WindowShown &event) {
        // windows.at(event.windowId)->show();
      }

      void onWindowHide(const Engine::Events::WindowHidden &event) {
        // windows.at(event.windowId)->hide();
      }

      void onWindowResize(const Engine::Events::WindowResized &event) {
        // Core::Application::Window &window = *windows.at(event.windowId);
        // window.resize(event.width, event.height);
        // LOG_CORE_INFO("Resize event processed and resized to {}x{}", event.width, event.height);
      }

      void onWindowMouseMotion(const Engine::Events::WindowMouseMotion &event) {
      }

      void onWindowMouseEnter(const Engine::Events::WindowMouseEnter &event) {
      }

      void onWindowMouseLeave(const Engine::Events::WindowMouseLeave &event) {
      }

      void onWindowFocusGained(const Engine::Events::WindowFocusGained &event) {
      }

      void onWindowFocusLost(const Engine::Events::WindowFocusLost &event) {
      }

      void onWindowMinimized(const Engine::Events::WindowMinimized &event) {
      }

      void onWindowMaximized(const Engine::Events::WindowMaximized &event) {
        Engine::Application::Window &window = *windows.at(event.windowId);
        window.maximize();
        if (renderer) {
          // renderer->recreateWindowSurface(window);
        }
      }

      void onWindowRestored(const Engine::Events::WindowRestored &event) {
        Engine::Application::Window &window = *windows.at(event.windowId);
        window.restore();
        if (renderer) {
          // renderer->recreateWindowSurface(window);
        }
      }

      void onWindowClose(const Engine::Events::WindowClosed &event) {
      }

      void onWindowExposed(const Engine::Events::WindowExposed &event) {
      }

      void onKeyPressed(const Engine::Events::KeyPressedEvent &event) {
      }

      void onKeyReleased(const Engine::Events::KeyReleasedEvent &event) {
      }

      void pollInputs() const;

      mutable std::unique_ptr<Engine::Renderer::Renderer> renderer;

      const char *name;
      mutable Engine::Events::EventDispatcher eventDispatcher;

      mutable std::vector<std::unique_ptr<Engine::Application::Layer>> layers;

      // Delta time in ms
      mutable std::uint64_t deltaTime = 0;
      mutable bool running = true;

      Engine::Application::DisplayInfo displayInfo = {};

      int displayCount = 0;
      mutable SDL_DisplayID *displays = nullptr;
      const SDL_DisplayMode *currentDisplay = nullptr;
      mutable std::unordered_map<std::uint32_t, std::unique_ptr<Engine::Application::Window>> windows;
  };
}
