#pragma once
#include "glm/vec3.hpp"

namespace Renderer {
  class AABB {
    public:
      AABB(const glm::vec3 &minimum, const glm::vec3 &maximum);

      const glm::vec3 &getMaximum() const { return maximum; }

      const glm::vec3 &getMinimum() const { return minimum; }

    private:
      glm::vec3 maximum;
      glm::vec3 minimum;
  };
}
