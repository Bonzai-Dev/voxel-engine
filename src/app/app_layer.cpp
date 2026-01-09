#include <glm/glm.hpp>
#include <core/renderer/renderer.hpp>
#include <util/graphics.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "game/world.hpp"
#include "app_layer.hpp"

AppLayer::AppLayer(const Core::Application &application) :
  RenderLayer(application), m_camera(application, glm::vec3(0.0f, 20.0f, 0.0f)) {
  m_shaderProgram.Use();

  for (size_t chunkCount = 0; chunkCount < Game::RenderDistance; chunkCount++) {
    const int x = chunkCount % Game::RenderDistance;
    const int z = (chunkCount / Game::RenderDistance) % Game::RenderDistance;
  }

  auto chunk1  = Game::Terrain::Chunk(glm::ivec2(0, 0));
  chunk1.index = 0;

  auto chunk2  = Game::Terrain::Chunk(glm::ivec2(1, 0));
  chunk2.index = 1;
  // auto chunk3  = Game::Terrain::Chunk(glm::ivec2(2, 0));
  // chunk3.index = 2;

  m_chunks.push_back(chunk1);
  m_chunks.push_back(chunk2);
  // m_chunks.push_back(chunk3);

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
