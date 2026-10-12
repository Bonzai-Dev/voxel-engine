#pragma once
#include <memory>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_mouse.h>
#include <core/application/layer.hpp>
#include <core/events/event.hpp>
#include <core/events/application_events.hpp>
#include <core/events/event.hpp>
#include <core/events/window_events.hpp>
#include <core/events/input_events.hpp>
#include <core/application/window.hpp>
#include <core/application/logger.hpp>
#include <../../core/include/core/renderer/renderer.hpp>

namespace Application {
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
        requires(std::is_base_of_v<Core::Application::Layer, LayerT>)
      void addLayer() const {
        layers.push_back(std::make_unique<LayerT>(*this));
      }

      void createWindow(const Core::Application::WindowOptions &options) const;

      const std::uint64_t &getDeltaTime() const { return deltaTime; }

      void quit() const;

      void run() const;

    private:
      void onWindowShow(const Core::Events::WindowShown &event) {
        windows.at(event.windowId)->show();
      }

      void onWindowHide(const Core::Events::WindowHidden &event) {
        windows.at(event.windowId)->hide();
      }

      void onWindowResize(const Core::Events::WindowResized &event) {
        Core::Application::Window &window = *windows.at(event.windowId);
        LOG_CORE_INFO("Window {} resized to {}x{}", event.windowId, event.width, event.height);
        window.resize(event.width, event.height);
      }

      void onWindowMouseMotion(const Core::Events::WindowMouseMotion &event) {
      }

      void onWindowMouseEnter(const Core::Events::WindowMouseEnter &event) {
      }

      void onWindowMouseLeave(const Core::Events::WindowMouseLeave &event) {
      }

      void onWindowFocusGained(const Core::Events::WindowFocusGained &event) {
      }

      void onWindowFocusLost(const Core::Events::WindowFocusLost &event) {
      }

      void onWindowMinimized(const Core::Events::WindowMinimized &event) {
      }

      void onWindowMaximized(const Core::Events::WindowMaximized &event) {
        Core::Application::Window &window = *windows.at(event.windowId);
        window.maximize();
        if (renderer) {
          // renderer->recreateWindowSurface(window);
        }
      }

      void onWindowRestored(const Core::Events::WindowRestored &event) {
        Core::Application::Window &window = *windows.at(event.windowId);
        window.restore();
        if (renderer) {
          // renderer->recreateWindowSurface(window);
        }
      }

      void onWindowClose(const Core::Events::WindowClosed &event) {
      }

      void onWindowExposed(const Core::Events::WindowExposed &event) {
      }

      void onKeyPressed(const Core::Events::KeyPressedEvent &event) {
      }

      void onKeyReleased(const Core::Events::KeyReleasedEvent &event) {
      }

      void pollInputs() const;

      Core::Application::Logger logger;
      
      mutable std::unique_ptr<Core::Renderer::Renderer> renderer;

      const char *name;
      mutable Core::Events::EventDispatcher eventDispatcher;

      mutable std::vector<std::unique_ptr<Core::Application::Layer>> layers;

      // Delta time in ms
      mutable std::uint64_t deltaTime = 0;
      mutable bool running = true;

      Core::Application::DisplayInfo displayInfo = {};

      int displayCount = 0;
      mutable SDL_DisplayID *displays = nullptr;
      const SDL_DisplayMode *currentDisplay = nullptr;
      mutable std::unordered_map<std::uint32_t, std::unique_ptr<Core::Application::Window>> windows;
  };
}
