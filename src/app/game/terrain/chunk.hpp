#pragma once
#include <glm/glm.hpp>
#include "blocks/block.hpp"

namespace Game::Terrain {
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
      Blocks::Block GetBlockLocal(const glm::ivec3 &position);

      void AddBlock(const Blocks::Block &block);

      bool OutOfBounds(const glm::ivec3 &position) const;

      bool FaceVisible(const glm::ivec3 &faceNormal, const glm::ivec3 &position);

      void AddFace(const glm::ivec3 &position, Blocks::Face face);

      unsigned int m_vertexArrayObject;
      unsigned int m_vertexBufferObject;
      unsigned int m_elementBufferObject;

      glm::mat4 m_modelMatrix = glm::mat4(1.0f);

      std::vector<float> m_vertexData;
      std::vector<unsigned int> m_indices;

      // 3D array flattened to 1D that stores all the chunk's block
      // Stores as 2 byte uint to save memory
      std::vector<std::uint16_t> m_blocks;
  };
}
