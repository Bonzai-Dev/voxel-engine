#include <glm/glm.hpp>
#include <core/renderer/renderer.hpp>
#include <util/graphics.hpp>
#include "game/shaders/default.hpp"
#include "game/terrain/blocks/block.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "app_layer.hpp"

AppLayer::AppLayer(const Core::Application &application) : RenderLayer(application), m_camera(application) {
  const Game::Chunk chunk;
  m_chunks.push_back(chunk);
}

void AppLayer::Render() {
  m_camera.Update();

  m_shaderProgram.Use();
  m_shaderProgram.UpdateProjectionMatrix(m_camera.GetProjectionMatrix());
  m_shaderProgram.UpdateViewMatrix(m_camera.GetViewMatrix());
  m_shaderProgram.UpdateModelMatrix(m_chunks[0].GetModelMatrix());

  Renderer::clearBuffer(Util::Graphics::normalizeColor(glm::vec4(0, 0, 0, 1)));
  m_chunks[0].Render();
}
