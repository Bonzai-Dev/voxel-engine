#pragma once
#include <glm/vec2.hpp>
#include <core/renderer/renderer.hpp>
#include <unordered_map>

#include "block.hpp"
#include "block_data.hpp"

namespace Game::Blocks {
  class BlockManager {
    public:
      BlockManager();

      void loadBlocks();

      void loadMesh(MeshId meshId) const;

      void serializeBlockData(BlockId blockId) const;

      void loadBlockMesh(BlockId blockId, MeshId meshId);

      glm::vec2 generateUv(const glm::ivec2 &spritesheetTile, const glm::vec3 &vertexPosition, Face face);

      const BlockMeshData &getBlockMeshData(BlockId blockId) const { return blockMeshData[blockId]; }

      const BlockData &getBlockData(BlockId blockId) const { return blockData[blockId]; }

      const MeshId &getBlockMeshId(BlockId blockId) const { return blockData[blockId].meshId; }

      const Renderer::MeshData &getMeshData(MeshId meshId) const { return meshData[meshId]; }

    private:
      mutable std::unordered_map<BlockId, BlockMeshData> blockMeshData;
      mutable std::unordered_map<MeshId, Renderer::MeshData> meshData;
      mutable std::unordered_map<BlockId, BlockData> blockData;
  };
}
