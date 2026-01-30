#include <glad/gl.h>
#include <core/renderer/renderer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "block.hpp"

using namespace Renderer;

namespace Game::Shader {
  Block::Block() : Shader("./res/shaders/default.vert", "./res/shaders/default.frag") {
  }

  void Block::updateModelMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("modelMatrix", program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Block::updateProjectionMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("projectionMatrix", program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Block::updateViewMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("viewMatrix", program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Block::updateTexture(unsigned int texture) const {
    const int location = Renderer::getUniform("imageTexture", program);
    glActiveTexture(GL_TEXTURE0);
    Renderer::useTexture(texture);
    glUniform1i(location, 0);
  }
}
