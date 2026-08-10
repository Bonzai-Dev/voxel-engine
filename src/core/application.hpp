#pragma once
#include <memory>
#include <core/logger.hpp>
#include <core/events/input_events.hpp>
#include "window.hpp"
#include "layer.hpp"

#if defined(ENGINE_COMPILER_MSVC)
  #define ENGINE_FORCE_INLINE __forceinline
#elif defined(ENGINE_COMPILER_GCC) && __GNUC__ > 3
  // Clang also defines __GNUC__ (as 4)
  #define ENGINE_FORCE_INLINE inline __attribute__ ((__always_inline__))
#else
  #define ENGINE_FORCE_INLINE inline
#endif

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

      const std::uint64_t &getDeltaTime() const { return deltaTime; }

      void quit() const;

      void run() const;

      // const Graphics::Backend graphicsBackend = selectGraphicsBackend();

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

      // Graphics::Backend selectGraphicsBackend() const;
      // mutable std::unique_ptr<Graphics::VulkanRenderingDevice> renderingDevice;

      Logger logger;

      const char *name;
      mutable Events::EventDispatcher eventDispatcher;

      mutable std::vector<std::unique_ptr<Layer> > layers;

      // Delta time in ms
      mutable std::uint64_t deltaTime = 0;
      mutable bool running = true;

      DisplayInfo displayInfo{};

      int displayCount = 0;
      mutable SDL_DisplayID *displays{};
      const SDL_DisplayMode *currentDisplay{};
      mutable std::unordered_map<std::uint32_t, RefCountedPtr<Window> > windows;
  };
}
