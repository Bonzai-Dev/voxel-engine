#pragma once
#include <array>
#include <glm/glm.hpp>
#include <core/renderer/aabb.hpp>

namespace Renderer {
  struct FrustumPlane {
    glm::vec4 transformation;

    float getDistanceToPoint(const glm::vec3 &point) const {
      return glm::dot(glm::vec3(transformation), point) + transformation.w;
    }
  };

  class Frustum {
    public:
      Frustum(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix);

      void update(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix);

      bool boundingBoxInView(const AABB &boundingBox) const;

    private:
      std::array<FrustumPlane, 6> planes {};
  };
}
