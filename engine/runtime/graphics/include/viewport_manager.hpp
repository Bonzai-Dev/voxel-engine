#pragma once
#include <engine/runtime/core/intrusive_ptr.hpp>

namespace Engine::Application {
  class GraphicsWindow;

  class ViewportManager {
    public:
      virtual ~ViewportManager() = default;

      virtual void createViewport(const SharedPtr<GraphicsWindow> &window) const = 0;

      virtual void render(const SharedPtr<GraphicsWindow> &window) const = 0;

    private:

  };
}