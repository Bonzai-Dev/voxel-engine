#pragma once
#include <memory>
#include <utility>
#include <vector>
#include <engine/runtime/events/event.hpp>
#include <engine/runtime/application/platform_application.hpp>

namespace Engine::Application {
  class UIRenderer;
}

namespace Engine::Scene {
  class SceneRenderer;
}

namespace Engine::RHI {
  struct Queue;
  struct SwapChain;
}

namespace Engine::Renderer {
  class RendererContext;
}

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

namespace Engine::Application {
  class GraphicsWindow;

  class GraphicsApplication {
    public:
      explicit GraphicsApplication(const SharedPtr<Renderer::RendererContext> &rendererContext);

      virtual ~GraphicsApplication();

      virtual SharedPtr<GraphicsWindow> addWindow(
        const SharedPtr<GraphicsWindow> &graphicalWindow,
        bool showImmediately
      ) const;

      void pollInputs() const;

    protected:
      SharedPtr<PlatformApplication> platformApplication;

    private:
      mutable Events::EventDispatcher eventDispatcher;

      mutable std::vector<SharedPtr<GraphicsWindow>> windows;

      SharedPtr<Renderer::RendererContext> rendererContext;
      mutable std::unique_ptr<UIRenderer> uiRenderer;

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
