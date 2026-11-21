#pragma once
#include <engine/runtime/core/intrusive_ptr.hpp>

namespace Engine::Scene {
  class SceneRenderer;

  class World {
    public:
      World();

    private:
      SharedPtr<SceneRenderer> sceneRenderer = nullptr;
  };
}
