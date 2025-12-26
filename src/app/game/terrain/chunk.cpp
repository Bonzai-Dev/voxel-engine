#include <glad/gl.h>
#include "chunk.hpp"

namespace Game {
  Chunk::Chunk() {
    glGenVertexArrays(1, &m_vertexArrayObject);
    glBindVertexArray(m_vertexArrayObject);

    glGenBuffers(1, &m_vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  }
}