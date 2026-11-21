#pragma once

namespace Engine::Application {
  class ViewportManager {
    public:
      virtual ~ViewportManager() = default;

      virtual void createViewport(const SharedPtr<GraphicsWindow> &window) const = 0;

      virtual void render(const SharedPtr<GraphicsWindow> &window) const = 0;

    private:

  };
}