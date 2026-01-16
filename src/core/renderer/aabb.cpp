#include "aabb.hpp"

namespace Renderer {
  AABB::AABB(const glm::vec3 &minimum, const glm::vec3 &maximum) :
  center((maximum + minimum) * 0.5f), extents(maximum.x - center.x, maximum.y - center.y, maximum.z - center.z) {
  }
}
