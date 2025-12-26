#pragma once
#include <memory>
#include <vector>
#include <core/window.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/constants.hpp>
#include <unordered_map>
#include "inputs.hpp"

namespace Core {
  class RenderLayer;

  class Application {
    public:
      explicit Application(const char *name);

      void Quit() const;

      void Run();

      void PollInputs();

      bool KeyDown(Inputs::KeyboardKey key, Inputs::KeyDetectMode detectMode) const;

      template<typename TLayer>
      requires(std::is_base_of_v<RenderLayer, TLayer>)
      void AddLayer() {
        m_layers.push_back(std::make_unique<TLayer>(*this));
      }

      double deltaTime = 0;
      glm::vec2 mouseDelta = glm::zero<glm::vec2>();
      bool mouseMoving = false;
      const std::shared_ptr<Window> window;

    private:
      std::vector<std::unique_ptr<RenderLayer> > m_layers;
      mutable std::unordered_map<Inputs::KeyboardKey, SDL_KeyboardEvent> m_pressedKeys;

      double m_lastFrameTime = 0;
      bool m_running;
  };
}
