#include <glad/gl.h>
#include "renderer.hpp"

namespace renderer {
  void clearBuffer(glm::vec4 color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
}