#pragma once
#include <vector>
#include "blocks/block.hpp"

namespace Game {
  class World;

  // Cannot build above MaxChunkHeight and below MinChunkHeight
  inline constexpr int MaxChunkHeight = 320;
  inline constexpr int MinChunkHeight = -64;
  inline constexpr int ChunkSize = 16;
  inline constexpr int ChunkHeight = MaxChunkHeight - MinChunkHeight;
  inline constexpr size_t TotalChunkBlocks = ChunkSize * ChunkSize * ChunkHeight;

  class Chunk {
    public:
      Chunk(const glm::ivec2 &position, const std::vector<int> &heightMap, World &world);

      ~Chunk() = default;

      void render();

      const glm::mat4 &getModelMatrix() const { return modelMatrix; }

      const glm::ivec2 &getPosition() const { return position; }

      bool meshLoaded() const { return loadedMesh; }

      void destroy() const;

    private:
      World &world;

      unsigned int vertexArrayObject;
      unsigned int vertexBufferObject;
      unsigned int elementBufferObject;

      glm::mat4 modelMatrix = glm::mat4(1.0f);
      glm::ivec2 position;
      bool loadedMesh = false;

      const std::vector<int> &heightMap;
      std::vector<float> vertexData;
      std::vector<unsigned int> indices;

      // 3D array flattened to 1D that stores all the chunk's block
      // Stores as 2 byte uint to save memory
      std::vector<std::uint16_t> blocks = std::vector(TotalChunkBlocks, static_cast<std::uint16_t>(Blocks::BlockId::Air));

      void loadMesh();

      void buildMesh();

      // Gets the block at the given local coordinates within the chunk
      // Origin of the chunk is located in (0, 0, 0)
      // Origin of the block is located at its bottom left corner
      Blocks::Block getBlockLocal(const glm::ivec3 &position);

      void addBlock(const Blocks::Block &block);

      static bool outOfBounds(const glm::ivec3 &position);

      bool faceVisible(const glm::ivec3 &faceNormal, const glm::ivec3 &position);

      void addFace(const glm::ivec3 &position, Blocks::Face face);

      // Gets height map using a 2d local position
      int getNoise(const glm::ivec2 &position) const;

      // Gets block index from the flattened 3d array
      static size_t getBlockIndex(const glm::ivec3 &position);
  };
}
