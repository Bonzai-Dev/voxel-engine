#pragma once
#include <glm/vec4.hpp>

namespace Util::Graphics {
  glm::vec4 normalizeColor(const glm::vec4 &color);

  glm::vec3 normalizeColor(const glm::vec3 &color);
}
