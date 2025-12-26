#include <glad/gl.h>
#include <glm/glm.hpp>
#include <core/renderer/renderer.hpp>
#include "game/shaders/default.hpp"
#include "app_layer.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "util/graphics.hpp"

static constexpr float vertices[] = {
  -0.5f, -0.5f, -0.5f,
  0.5f, -0.5f, -0.5f,
  0.5f, 0.5f, -0.5f,
  0.5f, 0.5f, -0.5f,
  -0.5f, 0.5f, -0.5f,
  -0.5f, -0.5f, -0.5f,

  -0.5f, -0.5f, 0.5f,
  0.5f, -0.5f, 0.5f,
  0.5f, 0.5f, 0.5f,
  0.5f, 0.5f, 0.5f,
  -0.5f, 0.5f, 0.5f,
  -0.5f, -0.5f, 0.5f,

  -0.5f, 0.5f, 0.5f,
  -0.5f, 0.5f, -0.5f,
  -0.5f, -0.5f, -0.5f,
  -0.5f, -0.5f, -0.5f,
  -0.5f, -0.5f, 0.5f,
  -0.5f, 0.5f, 0.5f,

  0.5f, 0.5f, 0.5f,
  0.5f, 0.5f, -0.5f,
  0.5f, -0.5f, -0.5f,
  0.5f, -0.5f, -0.5f,
  0.5f, -0.5f, 0.5f,
  0.5f, 0.5f, 0.5f,

  -0.5f, -0.5f, -0.5f,
  0.5f, -0.5f, -0.5f,
  0.5f, -0.5f, 0.5f,
  0.5f, -0.5f, 0.5f,
  -0.5f, -0.5f, 0.5f,
  -0.5f, -0.5f, -0.5f,

  -0.5f, 0.5f, -0.5f,
  0.5f, 0.5f, -0.5f,
  0.5f, 0.5f, 0.5f,
  0.5f, 0.5f, 0.5f,
  -0.5f, 0.5f, 0.5f,
  -0.5f, 0.5f, -0.5f,
};

auto modelMatrix = glm::mat4(1.0f);

AppLayer::AppLayer(const Core::Application &application) : RenderLayer(application),
  m_camera(application) {
  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -3.0f));

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);
}

void AppLayer::Render() {
  m_camera.Update();

  m_shaderProgram.Use();
  m_shaderProgram.UpdateProjectionMatrix(m_camera.projectionMatrix);
  m_shaderProgram.UpdateViewMatrix(m_camera.viewMatrix);
  m_shaderProgram.UpdateModelMatrix(modelMatrix);

  Renderer::clearBuffer(Util::normalizeColor(glm::vec4(76, 199, 255, 1)));
  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLES, 0, 36);
}
