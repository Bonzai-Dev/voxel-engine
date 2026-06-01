#pragma once
#include <memory>
#include <core/application/logger.hpp>
#include <core/events/application_events.hpp>
#include <core/renderer/vulkan/vulkan_rendering_device.hpp>
#include <core/renderer/vulkan/vulkan_window.hpp>
#include "layer.hpp"

namespace Core {
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

      void createWindow(const Graphics::WindowOptions &options) const;

      const double &getDeltaTime() const { return deltaTime; }

      void quit() const;

      void run() const;

      const Graphics::Backend graphicsBackend = selectGraphicsBackend();

    private:
      void pollInputs() const;

      Graphics::Backend selectGraphicsBackend() const;
      std::shared_ptr<Graphics::VulkanRenderingDevice> renderingDevice;

      Logger logger;

      const char *name;
      mutable Events::EventDispatcher eventDispatcher;

      mutable std::vector<std::unique_ptr<Layer>> layers;

      mutable double deltaTime = 0;
      mutable bool running = true;

      Graphics::DisplayInfo displayInfo{};

      int displayCount = 0;
      mutable SDL_DisplayID *displays{};
      const SDL_DisplayMode *currentDisplay{};
  };
}
