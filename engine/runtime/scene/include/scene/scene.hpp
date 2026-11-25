#pragma once
#include <memory>
#include <scene/scene_renderer.hpp>

namespace Engine::Scene {

  // Contains any cpu logic of the scene
  class Scene {
    public:
      Scene();

    private:
      std::unique_ptr<SceneRenderer> sceneRenderer;
  };
}
