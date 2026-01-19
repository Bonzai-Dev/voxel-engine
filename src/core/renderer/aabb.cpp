#include "aabb.hpp"

namespace Renderer {
  AABB::AABB(const glm::vec3 &minimum, const glm::vec3 &maximum) : minimum(minimum), maximum(maximum) {
  }
}
