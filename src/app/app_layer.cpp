#include <glad/gl.h>
#include <core/renderer/renderer.hpp>
#include "app_layer.hpp"

AppLayer::AppLayer() {
  const float vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f
  };

  const unsigned int shader = Renderer::createShaderProgram("./res/default.vert", "./res/default.frag");
  glUseProgram(shader);

  unsigned int vertexArrayObject;
  glGenVertexArrays(1, &vertexArrayObject);
  glBindVertexArray(vertexArrayObject);

  unsigned int vertexBufferObject;
  glGenBuffers(1, &vertexBufferObject);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);
}

void AppLayer::Render() {
  Renderer::clearBuffer(glm::vec4(76, 199, 255, 1) / 255.0f);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}
