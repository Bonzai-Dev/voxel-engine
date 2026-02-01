#include <glad/gl.h>
#include <core/renderer/renderer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "block.hpp"
#include "util/graphics.hpp"

using namespace Renderer;

namespace Game::Shader {
  Block::Block() : Shader("./res/shaders/block.vert", "./res/shaders/block.frag") {
  }

  void Block::setModelMatrix(const glm::mat4 &matrix) const {
    const int location = getUniform("modelMatrix", program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Block::setProjectionMatrix(const glm::mat4 &matrix) const {
    const int location = getUniform("projectionMatrix", program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Block::setViewMatrix(const glm::mat4 &matrix) const {
    const int location = getUniform("viewMatrix", program);
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(matrix));
  }

  void Block::updateTexture(unsigned int texture) const {
    const int location = getUniform("imageTexture", program);
    glActiveTexture(GL_TEXTURE0);
    useTexture(texture);
    glUniform1i(location, 0);
  }

  void Block::setAmbientColor(const glm::vec3 &color) const {
    const int location = getUniform("ambientLight", program);
    glUniform3fv(location, 1, glm::value_ptr(Util::Graphics::normalizeColor(glm::vec4(color, 1.0f))));
  }

  void Block::updateSun(const glm::vec3 &direction, const glm::vec3 &color, float brightness) const {
    setSunBrightness(brightness);
    setSunColor(color);
    setSunDirection(direction);
  }

  void Block::setSunDirection(const glm::vec3 &direction) const {
    const int location = getUniform("sun.direction", program);
    glUniform3fv(location, 1, glm::value_ptr(glm::normalize(direction)));
  }

  void Block::setSunColor(const glm::vec3 &color) const {
    const int location = getUniform("sun.color", program);
    glUniform3fv(location, 1, glm::value_ptr(Util::Graphics::normalizeColor(glm::vec4(color, 1.0f))));
  }

  void Block::setSunBrightness(float brightness) const {
    const int location = getUniform("sun.brightness", program);
    glUniform1f(location, brightness);
  }

  void Block::updateLinearFog(const glm::vec3 &color, float start, float end) const {
    setFogColor(color);
    setFogEquation(Linear);
    setFogStartEnd(start, end);
  }

  void Block::updateExponentialFog(const glm::vec3 &color, float density, bool squared) const {
    if (squared)
      setFogEquation(ExponentialSquared);
    else
      setFogEquation(Exponential);

    setFogColor(color);
    setFogDensity(density);
  }

  void Block::setFogColor(const glm::vec3 &color) const {
    const int location = getUniform("fog.color", program);
    glUniform3fv(location, 1, glm::value_ptr(Util::Graphics::normalizeColor(color)));
  }

  void Block::setFogEquation(FogEquation equation) const {
    const int location = getUniform("fog.equation", program);
    glUniform1i(location, static_cast<int>(equation));
  }

  void Block::setFogDensity(float density) const {
    const int location = getUniform("fog.density", program);
    glUniform1f(location, density);
  }

  void Block::setFogStartEnd(float start, float end) const {
    const int startLocation = getUniform("fog.start", program);
    const int endLocation = getUniform("fog.end", program);
    glUniform1f(startLocation, start);
    glUniform1f(endLocation, end);
  }
}
