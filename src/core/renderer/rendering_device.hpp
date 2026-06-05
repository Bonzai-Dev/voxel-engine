#pragma once
#include <core/events/window_events.hpp>
#include <core/events/input_events.hpp>
#include "window.hpp"

using namespace Core::Events;

namespace Core::Graphics {
  enum class Backend {
    None,
    DirectX11,
    DirectX12,
    Vulkan
  };

  class RenderingDevice {
    public:
      RenderingDevice(const char *appName, const DisplayInfo &displayInfo) :
      appName(appName), displayInfo(displayInfo) {
        EventDispatcher::listen<WindowShown>([&](const WindowShown &event) { onWindowShow(event); });
        EventDispatcher::listen<WindowHidden>([&](const WindowHidden &event) { onWindowHide(event); });
        EventDispatcher::listen<WindowResized>([&](const WindowResized &event) { onWindowResize(event); });

        EventDispatcher::listen<WindowMouseEnter>([&](const WindowMouseEnter &event) { onWindowMouseEnter(event); });
        EventDispatcher::listen<WindowMouseLeave>([&](const WindowMouseLeave &event) { onWindowMouseLeave(event); });
        EventDispatcher::listen<WindowMouseMotion>([&](const WindowMouseMotion &event) { onWindowMouseMotion(event); });

        EventDispatcher::listen<WindowFocusGained>([&](const WindowFocusGained &event) { onWindowFocusGained(event); });
        EventDispatcher::listen<WindowFocusLost>([&](const WindowFocusLost &event) { onWindowFocusLost(event); });
        EventDispatcher::listen<WindowMinimized>([&](const WindowMinimized &event) { onWindowMinimized(event); });
        EventDispatcher::listen<WindowMaximized>([&](const WindowMaximized &event) { onWindowMaximized(event); });
        EventDispatcher::listen<WindowRestored>([&](const WindowRestored &event) { onWindowRestored(event); });
        EventDispatcher::listen<WindowClosed>([&](const WindowClosed &event) { onWindowClose(event); });
        EventDispatcher::listen<WindowExposed>([&](const WindowExposed &event) { onWindowExposed(event); });

        EventDispatcher::listen<KeyPressedEvent>([&](const KeyPressedEvent &event) { onKeyPressed(event); });
        EventDispatcher::listen<KeyReleasedEvent>([&](const KeyReleasedEvent &event) { onKeyReleased(event); });
      }

      virtual ~RenderingDevice() = default;

      virtual void createWindow(const WindowOptions &options) {}

      virtual void render() {}

    private:
      void onWindowShow(const WindowShown &event) {
        windows.at(event.windowId).show();
      }

      void onWindowHide(const WindowHidden &event) {
        windows.at(event.windowId).hide();
      }

      void onWindowResize(const WindowResized &event) {
        windows.at(event.windowId).resize(event.width, event.height);
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

    protected:
      const char *appName;
      const DisplayInfo &displayInfo;
      std::unordered_map<std::uint32_t, Window> windows;
  };
}