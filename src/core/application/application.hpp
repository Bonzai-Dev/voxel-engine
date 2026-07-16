#pragma once
#include <memory>
#include <core/application/logger.hpp>
#include <core/events/input_events.hpp>
#include <core/rhi/vulkan/vulkan_rendering_device.hpp>
#include "window.hpp"
#include "layer.hpp"

namespace Core {
  using namespace Events;

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

      template<typename LayerT>
      requires(std::is_base_of_v<Layer, LayerT>)
      void addLayer() const {
        layers.push_back(std::make_unique<LayerT>(*this));
      }

      void createWindow(const WindowOptions &options) const;

      const double &getDeltaTime() const { return deltaTime; }

      void quit() const;

      void run() const;

      const Graphics::Backend graphicsBackend = selectGraphicsBackend();

    private:
      void onWindowShow(const WindowShown &event) {
        windows.at(event.windowId)->show();
      }

      void onWindowHide(const WindowHidden &event) {
        windows.at(event.windowId)->hide();
      }

      void onWindowResize(const WindowResized &event) {
        windows.at(event.windowId)->resize(event.width, event.height);
      }

      void onWindowMouseMotion(const WindowMouseMotion &event) {
      }

      void onWindowMouseEnter(const WindowMouseEnter &event) {
      }

      void onWindowMouseLeave(const WindowMouseLeave &event) {
      }

      void onWindowFocusGained(const WindowFocusGained &event) {
      }

      void onWindowFocusLost(const WindowFocusLost &event) {
      }

      void onWindowMinimized(const WindowMinimized &event) {
      }

      void onWindowMaximized(const WindowMaximized &event) {
      }

      void onWindowRestored(const WindowRestored &event) {
      }

      void onWindowClose(const WindowClosed &event) {
      }

      void onWindowExposed(const WindowExposed &event) {
      }

      void onKeyPressed(const KeyPressedEvent &event) {

      }

      void onKeyReleased(const KeyReleasedEvent &event) {
      }

      void pollInputs() const;

      Graphics::Backend selectGraphicsBackend() const;
      mutable std::unique_ptr<Graphics::VulkanRenderingDevice> renderingDevice;

      Logger logger;

      const char *name;
      mutable Events::EventDispatcher eventDispatcher;

      mutable std::vector<std::unique_ptr<Layer>> layers;

      mutable double deltaTime = 0;
      mutable bool running = true;

      DisplayInfo displayInfo{};

      int displayCount = 0;
      mutable SDL_DisplayID *displays{};
      const SDL_DisplayMode *currentDisplay{};
      mutable std::unordered_map<std::uint32_t, RefCountedPtr<Window>> windows;
  };
}
