#include <glm/vec3.hpp>
#include "graphics.hpp"

namespace Util::Graphics {
  glm::vec4 normalizeColor(const glm::vec4 &color) {
    return color / 255.0f;
  }

  glm::vec3 normalizeColor(const glm::vec3 &color) {
    return color / 255.0f;
  }
}
