#include <glm/glm.hpp>
#include <core/renderer/renderer.hpp>
#include <util/graphics.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "game/world.hpp"
#include "app_layer.hpp"

AppLayer::AppLayer(const Core::Application &application) :
  RenderLayer(application), camera(application, glm::vec3(0.0f, 20.0f, 0.0f)) {
  shaderProgram.use();

  const auto spritesheet = Renderer::loadPng("./res/images/blocks.png");
  shaderProgram.updateTexture(spritesheet);
}

void AppLayer::render() {
  camera.update();

  shaderProgram.use();
  shaderProgram.updateProjectionMatrix(camera.getProjectionMatrix());
  shaderProgram.updateViewMatrix(camera.getViewMatrix());

  Renderer::clearBuffer(Util::Graphics::normalizeColor(glm::vec4(0, 0, 0, 1)));
  for (unsigned int i = 0; i < chunks.size(); i++) {
    const auto &chunk = chunks[i];
    shaderProgram.updateModelMatrix(chunk.getModelMatrix());
    chunk.render();
  }
}
