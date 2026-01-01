#pragma once
#include <glm/glm.hpp>
#include "blocks/block.hpp"

namespace Game {
  class Chunk {
    public:
      Chunk();

      void Render() const;

      const glm::mat4 &GetModelMatrix() const { return m_modelMatrix; }

    private:
      void MeshBuilder();

      // Gets the block at the given local coordinates within the chunk
      // Origin of the chunk is located in (0, 0, 0)
      // Origin of the block is located at its bottom left corner
      Block GetBlockLocal(const glm::ivec3 &position);

      void AddBlockId(BlockId block);

      bool PositionInChunk(const glm::ivec3 &position) const;

      bool FaceVisible(const glm::ivec3 &faceNormal, const glm::ivec3 &position);

      void AddFace(const glm::ivec3 &position, Face face);

      unsigned int m_vertexArrayObject;
      unsigned int m_vertexBufferObject;
      unsigned int m_elementBufferObject;

      glm::mat4 m_modelMatrix = glm::mat4(1.0f);

      std::vector<float> m_vertexData;
      std::vector<unsigned int> m_indices;
      std::vector<std::uint16_t> m_blocks;
  };
}
