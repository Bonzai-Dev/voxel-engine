#pragma once
#include <glm/glm.hpp>
#include <core/renderer/shader.hpp>

namespace Game::Shader {
  class Default : public Renderer::Shader {
    public:
      Default();

      void UpdateModelMatrix(const glm::mat4 &matrix) const;

      void UpdateProjectionMatrix(const glm::mat4 &matrix) const;

      void UpdateViewMatrix(const glm::mat4 &matrix) const;

      void UpdateTexture(unsigned int texture) const;
  };
}
