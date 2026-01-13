#include <glm/glm.hpp>
#include <core/renderer/renderer.hpp>
#include <util/graphics.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "game/world.hpp"
#include "app_layer.hpp"

AppLayer::AppLayer(const Core::Application &application) :
  RenderLayer(application), m_camera(application, glm::vec3(0.0f, 20.0f, 0.0f)) {
  m_shaderProgram.Use();

  static const int gridSize = 5;

  for (int cz = 0; cz < gridSize; ++cz) {
    for (int cx = 0; cx < gridSize; ++cx) {
      m_chunks.emplace_back(glm::ivec2(cx, cz));
    }
  }

  const auto spritesheet = Renderer::loadPng("./res/images/blocks.png");
  m_shaderProgram.UpdateTexture(spritesheet);
}

void AppLayer::Render() {
  m_camera.Update();

  m_shaderProgram.Use();
  m_shaderProgram.UpdateProjectionMatrix(m_camera.GetProjectionMatrix());
  m_shaderProgram.UpdateViewMatrix(m_camera.GetViewMatrix());

  Renderer::clearBuffer(Util::Graphics::normalizeColor(glm::vec4(0, 0, 0, 1)));
  for (unsigned int i = 0; i < m_chunks.size(); i++) {
    const auto &chunk = m_chunks[i];
    m_shaderProgram.UpdateModelMatrix(chunk.GetModelMatrix());
    chunk.Render();
  }
}
