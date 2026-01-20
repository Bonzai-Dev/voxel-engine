#pragma once
#include <glm/vec2.hpp>
#include <core/renderer/renderer.hpp>
#include <unordered_map>
#include "block_data.hpp"

namespace Game::Blocks {
  enum class Face;

  struct BlockVertex : Renderer::MeshVertex {
    glm::vec2 uv;
  };

  using BlockVertexData = std::vector<BlockVertex>;

  struct BlockMeshData {
    BlockVertexData vertexData;
    Renderer::Indices indices;
  };

  struct BlockData {
    BlockId blockId;
    MeshId meshId;
    std::vector<glm::ivec2> spritesheetTiles;
  };

  class BlockManager {
    public:
      BlockManager();

      static void loadBlocks();

      static void loadMesh(MeshId meshId);

      static void serializeBlockData(BlockId blockId);

      static void loadBlockMesh(BlockId blockId, MeshId meshId);

      static glm::vec2 generateUv(const glm::ivec2 &spritesheetTile, const glm::vec3 &vertexPosition, Face face);

      static const BlockMeshData &getBlockMeshData(BlockId blockId) { return blockMeshData[blockId]; }

      static const BlockData &getBlockData(BlockId blockId) { return blockData[blockId]; }

      static MeshId getBlockMeshId(BlockId blockId) { return blockData[blockId].meshId; }

    private:
      static inline std::unordered_map<BlockId, BlockMeshData> blockMeshData;
      static inline std::unordered_map<MeshId, Renderer::MeshData> meshData;
      static inline std::unordered_map<BlockId, BlockData> blockData;
  };
}