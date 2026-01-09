#pragma once
#include <glm/glm.hpp>
#include <core/open_simplex2s.hpp>
#include "../world.hpp"
#include "blocks/block.hpp"

namespace Game::Terrain {
  class Chunk {
    public:
      Chunk(const glm::ivec2 &position);

      void Render() const;

      const glm::mat4 &GetModelMatrix() const { return m_modelMatrix; }
      int index = 0;
    private:
      static inline Core::OpenSimplexNoise::Noise m_noise{1};

      unsigned int m_vertexArrayObject;
      unsigned int m_vertexBufferObject;
      unsigned int m_elementBufferObject;

      glm::mat4 m_modelMatrix = glm::mat4(1.0f);
      glm::ivec2 m_position;

      std::vector<float> m_vertexData;
      std::vector<unsigned int> m_indices;

      // 3D array flattened to 1D that stores all the chunk's block
      // Stores as 2 byte uint to save memory
      std::vector<std::uint16_t> m_blocks = std::vector<std::uint16_t>(TotalChunkBlocks, 0);

      void MeshBuilder();

      // Gets the block at the given local coordinates within the chunk
      // Origin of the chunk is located in (0, 0, 0)
      // Origin of the block is located at its bottom left corner
      Blocks::Block GetBlockLocal(const glm::ivec3 &position);

      void AddBlock(const Blocks::Block &block);

      static bool OutOfBounds(const glm::ivec3 &position) ;

      bool FaceVisible(const glm::ivec3 &faceNormal, const glm::ivec3 &position);

      void AddFace(const glm::ivec3 &position, Blocks::Face face);

      // Gets block index from the flattened 3d array
      static size_t GetBlockIndex(const glm::ivec3 &position);
  };
}
