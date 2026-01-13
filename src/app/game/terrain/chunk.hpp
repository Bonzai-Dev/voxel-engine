#pragma once
#include <glm/glm.hpp>
#include <core/open_simplex2s.hpp>
#include "blocks/block.hpp"

namespace Game::Terrain {
  // Cannot build above MaxChunkHeight and below MinChunkHeight
  inline constexpr int MaxChunkHeight = 320;
  inline constexpr int MinChunkHeight = -64;
  inline constexpr int ChunkSize = 16;
  inline constexpr int ChunkHeight = MaxChunkHeight - MinChunkHeight;
  inline constexpr size_t TotalChunkBlocks = ChunkSize * ChunkSize * ChunkHeight;

  class Chunk {
    public:
      Chunk(const glm::ivec2 &position);

      void render() const;

      const glm::mat4 &getModelMatrix() const { return modelMatrix; }

    private:
      static inline Core::OpenSimplexNoise::Noise noise{1};

      unsigned int vertexArrayObject;
      unsigned int vertexBufferObject;
      unsigned int elementBufferObject;

      glm::mat4 modelMatrix = glm::mat4(1.0f);
      glm::ivec2 position;

      std::vector<float> vertexData;
      std::vector<unsigned int> indices;

      // 3D array flattened to 1D that stores all the chunk's block
      // Stores as 2 byte uint to save memory
      std::vector<std::uint16_t> blocks = std::vector<std::uint16_t>(TotalChunkBlocks, 0);

      void buildMesh();

      // Gets the block at the given local coordinates within the chunk
      // Origin of the chunk is located in (0, 0, 0)
      // Origin of the block is located at its bottom left corner
      Blocks::Block getBlockLocal(const glm::ivec3 &position);

      void addBlock(const Blocks::Block &block);

      static bool outOfBounds(const glm::ivec3 &position) ;

      bool faceVisible(const glm::ivec3 &faceNormal, const glm::ivec3 &position);

      void addFace(const glm::ivec3 &position, Blocks::Face face);

      // Gets block index from the flattened 3d array
      static size_t getBlockIndex(const glm::ivec3 &position);
  };
}
