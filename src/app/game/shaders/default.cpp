#include <glad/gl.h>
#include <core/renderer/renderer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "default.hpp"

namespace Game::Shader {
  DefaultShader::DefaultShader() : Shader("./res/shaders/default.vert", "./res/shaders/default.frag") {
  }

  void DefaultShader::UpdateModelMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("modelMatrix", m_program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void DefaultShader::UpdateProjectionMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("projectionMatrix", m_program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void DefaultShader::UpdateViewMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("viewMatrix", m_program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }
}
