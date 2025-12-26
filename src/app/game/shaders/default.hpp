#pragma once
#include <glm/glm.hpp>
#include "core/renderer/shader.hpp"

namespace Game::Shader {
  class DefaultShader : public Renderer::Shader {
    public:
      DefaultShader();

      void UpdateModelMatrix(const glm::mat4 &matrix) const;

      void UpdateProjectionMatrix(const glm::mat4 &matrix) const;

      void UpdateViewMatrix(const glm::mat4 &matrix) const;
  };
}
