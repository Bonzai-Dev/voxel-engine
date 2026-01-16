#pragma once

#include <glm/vec3.hpp>

namespace Renderer {
  class FrustumPlane {
    public:
      FrustumPlane(const glm::vec3 &normal, float distance);

    private:
      glm::vec3 normal;

  };

  struct Frustum {
    FrustumPlane top;
    FrustumPlane bottom;
    FrustumPlane left;
    FrustumPlane right;
    FrustumPlane far;
    FrustumPlane near;
  };
}