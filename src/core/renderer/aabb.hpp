#pragma once
#include "glm/vec3.hpp"

namespace Renderer {
  class AABB {
    public:
      AABB(const glm::vec3 &minimum, const glm::vec3 &maximum);

      const glm::vec3 &getCenter() const { return center; }

      const glm::vec3 &getExtents() const { return extents; }

    private:
      glm::vec3 center;
      glm::vec3 extents;
  };
}
