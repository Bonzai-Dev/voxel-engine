#pragma once
#include <glm/glm.hpp>
#include <core/renderer/shader.hpp>

namespace Game::Shader {
  enum FogEquation {
    Linear = 0,
    Exponential = 1,
    ExponentialSquared = 2
  };

  class Block : public Renderer::Shader {
    public:
      Block();

      void setModelMatrix(const glm::mat4 &matrix) const;

      void setProjectionMatrix(const glm::mat4 &matrix) const;

      void setViewMatrix(const glm::mat4 &matrix) const;

      void updateTexture(unsigned int texture) const;

      void setAmbientColor(const glm::vec3 &color) const;

      void updateSun(const glm::vec3 &direction, const glm::vec3 &color, float brightness) const;

      void setSunDirection(const glm::vec3 &direction) const;

      void setSunColor(const glm::vec3 &color) const;

      void setSunBrightness(float brightness) const;

      void updateLinearFog(const glm::vec3 &color, float start, float end) const;

      void updateExponentialFog(const glm::vec3 &color, float density, bool squared) const;

      void setFogColor(const glm::vec3 &color) const;

      void setFogEquation(FogEquation equation) const;

      void setFogDensity(float density) const;

      void setFogStartEnd(float start, float end) const;
  };
}
