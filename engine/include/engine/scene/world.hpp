#pragma once
#include <engine/memory/intrusive_ptr.hpp>

namespace Engine::Renderer {
  class SceneRenderer;
}

namespace Engine::World {
  class World {
    public:
      World();

    private:
      SharedPtr<Renderer::SceneRenderer> sceneRenderer = nullptr;
  };
}
