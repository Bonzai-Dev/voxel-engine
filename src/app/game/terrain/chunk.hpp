#pragma once
#include <glm/glm.hpp>

namespace Game {
  class Chunk {
    public:
      Chunk();

    private:
      unsigned int m_vertexArrayObject;
      unsigned int m_vertexBufferObject;
      glm::mat4 m_modelMatrix = glm::mat4(1.0f);
  };
}
