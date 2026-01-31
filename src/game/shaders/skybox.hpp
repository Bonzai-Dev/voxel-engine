#pragma once
#include <glm/glm.hpp>
#include "block.hpp"

namespace Game::Shader {
  class Skybox : public Renderer::Shader {
    public:
      Skybox();

      void setProjectionMatrix(const glm::mat4 &matrix) const;

      void setViewMatrix(const glm::mat4 &matrix) const;

      void updateTexture(unsigned int texture) const;
  };
}
