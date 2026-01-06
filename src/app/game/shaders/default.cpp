#include <glad/gl.h>
#include <core/renderer/renderer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "default.hpp"

namespace Game::Shader {
  Default::Default() : Shader("./res/shaders/default.vert", "./res/shaders/default.frag") {
  }

  void Default::UpdateModelMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("modelMatrix", m_program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Default::UpdateProjectionMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("projectionMatrix", m_program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Default::UpdateViewMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("viewMatrix", m_program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Default::UpdateTexture(unsigned int texture) const {
    const int location = Renderer::getUniform("imageTexture", m_program);
    glActiveTexture(GL_TEXTURE0);
    Renderer::useTexture(texture);
    glUniform1i(location, 0);
  }
}
