#pragma once
#include <glm/glm.hpp>

namespace Renderer {
  class FrustumPlane {
    public:
      FrustumPlane() = default;

      glm::vec3 normal{};
      float distance = 0;

      bool onPlane(const glm::vec3 &position) const {
        return getSignedDistance(position) >= 0;
      }

    private:
      float getSignedDistance(const glm::vec3 &position) const {
        return glm::dot(normal, position) - distance;
      }
  };
}
