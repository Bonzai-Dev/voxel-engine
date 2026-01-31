#pragma once
#include <glm/glm.hpp>
#include <core/renderer/shader.hpp>

namespace Game::Shader {
  class Block : public Renderer::Shader {
    public:
      Block();

      void setModelMatrix(const glm::mat4 &matrix) const;

      void setProjectionMatrix(const glm::mat4 &matrix) const;

      void setViewMatrix(const glm::mat4 &matrix) const;

      void updateTexture(unsigned int texture) const;

      void setAmbientColor(const glm::vec3 &color) const;

      void setSunDirection(const glm::vec3 &direction) const;

      void setSunColor(const glm::vec3 &color) const;
  };
}
