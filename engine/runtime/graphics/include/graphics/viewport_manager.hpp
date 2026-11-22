#pragma once
#include <core/shared_pointer.hpp>

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