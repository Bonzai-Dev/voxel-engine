#pragma once
#include <glm/glm.hpp>
#include <core/renderer/aabb.hpp>
#include <iostream>

namespace Renderer {
  struct FrustumPlane {
    glm::vec4 transformation;

    float getDistanceToPoint(const glm::vec3 &point) const {
      float distance = transformation.x * point.x + transformation.y * point.y + transformation.z * point.z + transformation.w;
      std::cout << "normal : " << transformation.x << ", " << transformation.y << ", " << transformation.z << ", " << transformation.w << std::endl;
      // std::cout << "distance: " << glm::dot(transformation, glm::vec4(point, 1.0f)) << std::endl;
      return glm::dot(glm::vec3(transformation), point) + transformation.w;
      // std::cout << "distance: " << distance << std::endl;
      // return distance;
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
