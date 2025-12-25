#pragma once
#include <memory>
#include <vector>
#include <core/window.hpp>
#include "render_layer.hpp"

namespace Core {
  class Application {
    public:
      explicit Application(const char *name);

      void Quit() const;

      void Run();

      template<typename TLayer>
      requires(std::is_base_of_v<RenderLayer, TLayer>)
      void AddLayer() {
        m_layers.push_back(std::make_unique<TLayer>());
      }

      double deltaTime = 0;

    private:
      std::unique_ptr<Window> m_window;
      std::vector<std::unique_ptr<RenderLayer>> m_layers;

      double m_lastFrameTime = 0;
      bool m_running;
      const char *m_name;
  };
}
