#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <core/renderer/renderer.hpp>
#include "skybox.hpp"

namespace Game::Shader {
  Skybox::Skybox() : Shader("./res/shaders/skybox.vert", "./res/shaders/skybox.frag") {
  }

  void Skybox::updateProjectionMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("projectionMatrix", program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Skybox::updateViewMatrix(const glm::mat4 &matrix) const {
    const int location = Renderer::getUniform("viewMatrix", program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Skybox::updateTexture(unsigned int texture) const {
    const int location = Renderer::getUniform("cubeMap", program);
    glActiveTexture(GL_TEXTURE1);
    Renderer::useCubeMap(texture);
    glUniform1i(location, 1);
  }
}
