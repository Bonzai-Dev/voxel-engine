#include <glad/gl.h>
#include <core/renderer/renderer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "default.hpp"

namespace Game::Shader {
  Default::Default() : Shader("./res/shaders/default.vert", "./res/shaders/default.frag") {
  }

  void Default::updateModelMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("modelMatrix", program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Default::updateProjectionMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("projectionMatrix", program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Default::updateViewMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("viewMatrix", program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Default::updateTexture(unsigned int texture) const {
    const int location = Renderer::getUniform("imageTexture", program);
    glActiveTexture(GL_TEXTURE0);
    Renderer::useTexture(texture);
    glUniform1i(location, 0);
  }
}
