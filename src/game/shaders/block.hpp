#pragma once
#include <glm/glm.hpp>
#include <core/renderer/shader.hpp>

namespace Game::Shader {
  class Block : public Renderer::Shader {
    public:
      Block();

      void updateModelMatrix(const glm::mat4 &matrix) const;

      void updateProjectionMatrix(const glm::mat4 &matrix) const;

      void updateViewMatrix(const glm::mat4 &matrix) const;

      void updateTexture(unsigned int texture) const;

      void updateAmbientLight(const glm::vec3 &color) const;
  };
}
