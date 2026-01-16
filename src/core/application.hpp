#pragma once
#include <memory>
#include <vector>
#include <renderer/window.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/constants.hpp>
#include <unordered_map>
#include "inputs.hpp"

namespace Core {
  class RenderLayer;

  class Application {
  public:
    explicit Application(const char *name);

    void quit() const;

    void run();

    void pollInputs();

    bool keyDown(Inputs::KeyboardKey key, Inputs::KeyDetectMode detectMode) const;

    template<typename LayerT>
    requires(std::is_base_of_v<RenderLayer, LayerT>)
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

    std::vector<std::unique_ptr<RenderLayer> > layers;
    mutable std::unordered_map<Inputs::KeyboardKey, SDL_KeyboardEvent> pressedKeys;

    double lastFrameTime = 0;
    bool running;
  };
}
