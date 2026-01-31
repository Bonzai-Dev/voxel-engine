#include <glad/gl.h>
#include <core/renderer/renderer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "block.hpp"

#include "util/graphics.hpp"

using namespace Renderer;

namespace Game::Shader {
  Block::Block() : Shader("./res/shaders/block.vert", "./res/shaders/block.frag") {
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

  void Block::updateAmbientLight(const glm::vec3 &color) const {
    const int location = Renderer::getUniform("ambientLight", program);
    glUniform3fv(location, 1, glm::value_ptr(Util::Graphics::normalizeColor(glm::vec4(color, 1.0f))));
  }
}
