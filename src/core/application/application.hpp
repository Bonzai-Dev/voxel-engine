#pragma once
#include <unordered_map>
#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/gtc/constants.hpp>
#include "render_layer.hpp"
#include "window.hpp"
#include "inputs.hpp"

namespace Core {
  class Application {
    public:
      explicit Application(const char *name);

      void quit() const;

      void run();

      void pollInputs();

      bool keyDown(Inputs::KeyboardKey key, Inputs::KeyDetectMode detectMode) const;

      template<typename LayerT>
      requires(std::is_base_of_v<Core::RenderLayer, LayerT>)
      void addLayer() {
        layers.push_back(std::make_unique<LayerT>(*this));
      }

      const double &getDeltaTime() const { return deltaTime; }

      const glm::vec2 &getMouseDelta() const { return mouseDelta; }

      const std::shared_ptr<Window> &getWindow() const { return window; }

      const bool &mouseMoving() const { return isMouseMoving; }

    private:
      double deltaTime = 0;
      glm::vec2 mouseDelta = glm::zero<glm::vec2>();
      bool isMouseMoving = false;
      const std::shared_ptr<Window> window;

      std::vector<std::unique_ptr<Core::RenderLayer> > layers;
      mutable std::unordered_map<Inputs::KeyboardKey, SDL_KeyboardEvent> pressedKeys;

      double lastFrameTime = 0;
      bool running;
  };
}
