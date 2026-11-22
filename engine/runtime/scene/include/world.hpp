#pragma once
#include <memory>
#include <engine/runtime/scene/scene_renderer.hpp>

namespace Engine::Scene {
  class World {
    public:
      World();

    private:
      std::unique_ptr<SceneRenderer> sceneRenderer;
  };
}
